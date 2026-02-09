// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"

#include "CMGameplayTags.h"
#include "GameplayEffect.h"
#include "GameplayAbilitySystem/CMAttributeSet.h"
#include "GameplayTags/CMGameplayTags_Currency.h"

// 디버그 로그 활성화 플래그 (콘솔 변수로 제어 가능)
static TAutoConsoleVariable<bool> CVarCurrencyDebugLog(
	TEXT("cm.Currency.DebugLog"),
	true,
	TEXT("재화 변경 디버그 로그 활성화/비활성화\n")
	TEXT("0: 비활성화\n")
	TEXT("1: 활성화 (기본값)"),
	ECVF_Default
	);

void UCMAbilitySystemComponent::ServerAddLooseGameplayTag_Implementation(FGameplayTag TagToAdd)
{
	if (!HasMatchingGameplayTag(TagToAdd))
	{
		AddLooseGameplayTag(TagToAdd);
	}
}

void UCMAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		if (InputTag.MatchesTag(CMGameplayTags::InputTag_Toggleable) && AbilitySpec.IsActive())
		{
			CancelAbilityHandle(AbilitySpec.Handle);
		}
		else
		{
			TryActivateAbility(AbilitySpec.Handle);
		}
	}
}

void UCMAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid() || !InputTag.MatchesTag(CMGameplayTags::InputTag_MustBeHeld))
	{
		return;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			// InputReleased 이벤트를 어빌리티에 전달 (취소하지 않음)
			FGameplayAbilitySpec* MutableSpec = FindAbilitySpecFromHandle(AbilitySpec.Handle);
			if (MutableSpec)
			{
				AbilitySpecInputReleased(*MutableSpec);
			}
		}
	}
}

#pragma region Currency System

void UCMAbilitySystemComponent::AddCurrency(int32 Amount, FGameplayTag ReasonTag, UObject* Source)
{
	// 음수 값은 무시 (SpendCurrency 사용 권장)
	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] AddCurrency: 0 이하의 값으로 호출됨: %d"), Amount);
		return;
	}

	// 서버 권한 체크
	AActor* LocalOwnerActor = GetOwnerActor();
	if (LocalOwnerActor && LocalOwnerActor->HasAuthority())
	{
		// 서버에서 직접 처리
		Internal_ModifyCurrency(Amount, ReasonTag);
	}
	else
	{
		// 클라이언트에서 호출 시 Server RPC 사용
		ServerAddCurrency(Amount, ReasonTag);
	}
}

void UCMAbilitySystemComponent::SpendCurrency(int32 Cost, FGameplayTag ReasonTag, bool& bSuccess)
{
	bSuccess = false;

	// 음수 값은 무시
	if (Cost <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] SpendCurrency: 0 이하의 비용으로 호출됨: %d"), Cost);
		return;
	}

	// 서버 권한 체크
	AActor* LocalOwnerActor = GetOwnerActor();
	if (LocalOwnerActor && LocalOwnerActor->HasAuthority())
	{
		// 잔액 확인
		if (!CanAfford(Cost))
		{
			UE_LOG(LogTemp, Warning, TEXT("[재화] SpendCurrency 실패: 잔액 부족. 비용=%d, 현재=%d"),
				Cost, GetCurrentCurrencyAmount());
			return;
		}

		// 서버에서 직접 처리 (음수로 변환)
		bSuccess = Internal_ModifyCurrency(-Cost, ReasonTag);
	}
	else
	{
		// 클라이언트에서 호출 시 Server RPC 사용
		// 클라이언트는 잔액을 로컬에서 확인할 수 있지만 최종 결정은 서버에서
		if (CanAfford(Cost))
		{
			ServerSpendCurrency(Cost, ReasonTag);
			// 클라이언트에서는 성공 여부를 즉시 알 수 없음 (서버 응답 대기 필요)
			// UI는 OnCurrencyChanged 델리게이트로 갱신됨
			bSuccess = true; // 낙관적 응답 (서버에서 실패하면 Replication으로 원복됨)
		}
	}
}

bool UCMAbilitySystemComponent::CanAfford(int32 Cost) const
{
	return GetCurrentCurrencyAmount() >= Cost;
}

int32 UCMAbilitySystemComponent::GetCurrentCurrencyAmount() const
{
	// AttributeSet에서 CurrentCurrency 값을 가져옴
	const UCMAttributeSet* AttributeSet = GetSet<UCMAttributeSet>();
	if (AttributeSet)
	{
		// float을 정수로 변환 (항상 RoundToInt 사용)
		return FMath::RoundToInt(AttributeSet->GetCurrentCurrency());
	}
	return 0;
}

void UCMAbilitySystemComponent::ServerAddCurrency_Implementation(int32 Amount, FGameplayTag ReasonTag)
{
	// 서버에서 실행됨
	if (Amount > 0)
	{
		Internal_ModifyCurrency(Amount, ReasonTag);
	}
}

void UCMAbilitySystemComponent::ServerSpendCurrency_Implementation(int32 Cost, FGameplayTag ReasonTag)
{
	// 서버에서 실행됨
	if (Cost > 0 && CanAfford(Cost))
	{
		Internal_ModifyCurrency(-Cost, ReasonTag);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] ServerSpendCurrency 실패: 잔액 부족 또는 잘못된 비용. 비용=%d, 현재=%d"),
			Cost, GetCurrentCurrencyAmount());
	}
}

bool UCMAbilitySystemComponent::Internal_ModifyCurrency(int32 DeltaAmount, FGameplayTag ReasonTag)
{
	// 서버에서만 실행되어야 함
	AActor* LocalOwnerActor = GetOwnerActor();
	if (!LocalOwnerActor || !LocalOwnerActor->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("[재화] Internal_ModifyCurrency: 서버가 아닌 곳에서 호출됨!"));
		return false;
	}

	const UCMAttributeSet* AttributeSet = GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("[재화] Internal_ModifyCurrency: AttributeSet을 찾을 수 없음!"));
		return false;
	}

	const int32 OldAmount = FMath::RoundToInt(AttributeSet->GetCurrentCurrency());

	// 값이 변하지 않으면 early return
	if (DeltaAmount == 0)
	{
		return false;
	}

	// GameplayEffect를 사용하여 재화 변경
	const bool bApplied = ApplyCurrencyModifyEffect(DeltaAmount);
	if (!bApplied)
	{
		UE_LOG(LogTemp, Error, TEXT("[재화] Internal_ModifyCurrency: GE 적용 실패!"));
		return false;
	}

	// GE 적용 후 새 값 확인
	const int32 NewAmount = FMath::RoundToInt(AttributeSet->GetCurrentCurrency());
	const int32 ActualDelta = NewAmount - OldAmount;

	// 디버그 로그 출력
	const bool bIsGain = ActualDelta > 0;
	LogCurrencyChange(OldAmount, NewAmount, ActualDelta, ReasonTag, bIsGain);

	// 델리게이트 브로드캐스트
	if (OnCurrencyChanged.IsBound())
	{
		OnCurrencyChanged.Broadcast(NewAmount, ActualDelta, ReasonTag);
	}

	return true;
}

void UCMAbilitySystemComponent::LogCurrencyChange(int32 OldAmount, int32 NewAmount, int32 DeltaAmount, const FGameplayTag& ReasonTag, bool bIsGain) const
{
	// 콘솔 변수로 로그 활성화 여부 확인
	if (!CVarCurrencyDebugLog.GetValueOnGameThread())
	{
		return;
	}

	const AActor* LocalOwnerActor = GetOwnerActor();
	const FString OwnerName = LocalOwnerActor ? LocalOwnerActor->GetName() : TEXT("알 수 없음");
	const FString ReasonString = ReasonTag.IsValid() ? ReasonTag.ToString() : TEXT("없음");
	const FString ActionString = bIsGain ? TEXT("획득") : TEXT("소비");
	const int32 AbsDelta = FMath::Abs(DeltaAmount);

	UE_LOG(LogTemp, Log, TEXT("[재화] %s: %d %s (사유: %s) | 잔액: %d -> %d"),
		*OwnerName, AbsDelta, *ActionString, *ReasonString, OldAmount, NewAmount);
}

void UCMAbilitySystemComponent::EnsureCurrencyEffectClassInitialized()
{
	if (CurrencyModifyEffectClass)
	{
		return;
	}

	// 런타임에 GameplayEffect 클래스 생성
	CurrencyModifyEffectClass = NewObject<UClass>(
		GetTransientPackage(),
		UGameplayEffect::StaticClass()->GetFName(),
		RF_Transient
		);
}

bool UCMAbilitySystemComponent::ApplyCurrencyModifyEffect(int32 DeltaAmount)
{
	if (DeltaAmount == 0)
	{
		return false;
	}

	// AttributeSet 확인
	const UCMAttributeSet* AttributeSet = GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("[재화] ApplyCurrencyModifyEffect: AttributeSet을 찾을 수 없음!"));
		return false;
	}

	// 새 값 계산
	const float CurrentValue = AttributeSet->GetCurrentCurrency();
	const float NewValue = CurrentValue + static_cast<float>(DeltaAmount);

	// GE Context 생성
	FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
	ContextHandle.AddSourceObject(GetOwnerActor());

	// 런타임에 GE 인스턴스 생성 (Override 방식)
	UGameplayEffect* CurrencyEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_CurrencyModify_Runtime")));
	CurrencyEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

	// Modifier 설정: Override로 새 값 직접 설정
	FGameplayModifierInfo& ModifierInfo = CurrencyEffect->Modifiers.AddDefaulted_GetRef();
	ModifierInfo.Attribute = UCMAttributeSet::GetCurrentCurrencyAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Override;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(NewValue));

	// GE Spec 생성 및 적용
	FGameplayEffectSpec Spec(CurrencyEffect, ContextHandle, 1.0f);
	ApplyGameplayEffectSpecToSelf(Spec);

	// Instant GE는 즉시 완료되므로 ActiveHandle이 invalid일 수 있음
	// 적용 후 값이 변경되었는지 확인하여 성공 여부 판단
	const float AfterValue = AttributeSet->GetCurrentCurrency();
	const bool bSuccess = !FMath::IsNearlyEqual(CurrentValue, AfterValue) || FMath::IsNearlyEqual(NewValue, AfterValue);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] ApplyCurrencyModifyEffect: GE 적용 후 값 변경 없음. Before=%.0f, Expected=%.0f, After=%.0f"),
			CurrentValue, NewValue, AfterValue);
	}

	return true;
}

#pragma endregion