#include "Components/UI/UPawnUIComponent.h"
#include "Character/CMCharacterBase.h"
#include "GameplayAbilitySystem/CMAttributeSet.h"
#include "Net/UnrealNetwork.h"

UPawnUIComponent::UPawnUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPawnUIComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPawnUIComponent, AbilityIcons);
}

void UPawnUIComponent::InitializeWithASC(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	CachedASC = ASC;

	const UCMAttributeSet* AttributeSet = ASC->GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		return;
	}

	// ASC의 속성 변경 델리게이트 구독 (서버/클라이언트 모두 자동으로 작동)
	HealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCurrentHealthAttribute()).AddUObject(this, &ThisClass::OnHealthAttributeChanged);
	MaxHealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).AddUObject(this, &ThisClass::OnMaxHealthAttributeChanged);
	ManaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCurrentManaAttribute()).AddUObject(this, &ThisClass::OnManaAttributeChanged);
	MaxManaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxManaAttribute()).AddUObject(this, &ThisClass::OnMaxManaAttributeChanged);
	StaminaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCurrentStaminaAttribute()).AddUObject(this, &ThisClass::OnStaminaAttributeChanged);
	MaxStaminaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxStaminaAttribute()).AddUObject(this, &ThisClass::OnMaxStaminaAttributeChanged);

	// 재화 변경 델리게이트 구독
	CurrencyChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCurrentCurrencyAttribute()).AddUObject(this, &ThisClass::OnCurrencyAttributeChanged);

	// 초기 재화 캐싱
	CachedCurrencyAmount = FMath::RoundToInt(AttributeSet->GetCurrentCurrency());
}

void UPawnUIComponent::BeginDestroy()
{
	// 델리게이트 언바인딩
	if (CachedASC.IsValid())
	{
		if (HealthChangedDelegateHandle.IsValid())
		{
			CachedASC->GetGameplayAttributeValueChangeDelegate(UCMAttributeSet::GetCurrentHealthAttribute()).Remove(HealthChangedDelegateHandle);
		}

		if (MaxHealthChangedDelegateHandle.IsValid())
		{
			CachedASC->GetGameplayAttributeValueChangeDelegate(UCMAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedDelegateHandle);
		}

		if (ManaChangedDelegateHandle.IsValid())
		{
			CachedASC->GetGameplayAttributeValueChangeDelegate(UCMAttributeSet::GetCurrentManaAttribute()).Remove(ManaChangedDelegateHandle);
		}

		if (MaxManaChangedDelegateHandle.IsValid())
		{
			CachedASC->GetGameplayAttributeValueChangeDelegate(UCMAttributeSet::GetMaxManaAttribute()).Remove(MaxManaChangedDelegateHandle);
		}

		if (StaminaChangedDelegateHandle.IsValid())
		{
			CachedASC->GetGameplayAttributeValueChangeDelegate(UCMAttributeSet::GetCurrentStaminaAttribute()).Remove(StaminaChangedDelegateHandle);
		}

		if (MaxStaminaChangedDelegateHandle.IsValid())
		{
			CachedASC->GetGameplayAttributeValueChangeDelegate(UCMAttributeSet::GetMaxStaminaAttribute()).Remove(MaxStaminaChangedDelegateHandle);
		}

		if (CurrencyChangedDelegateHandle.IsValid())
		{
			CachedASC->GetGameplayAttributeValueChangeDelegate(UCMAttributeSet::GetCurrentCurrencyAttribute()).Remove(CurrencyChangedDelegateHandle);
		}
	}

	Super::BeginDestroy();
}

void UPawnUIComponent::BroadcastInitStatusValues()
{
	ACMCharacterBase* Character = GetOwningPawn<ACMCharacterBase>();
	if (!IsValid(Character))
	{
		return;
	}

	const UCMAttributeSet* AttributeSet = Character->GetCMAttributeSet();
	if (!IsValid(AttributeSet))
	{
		return;
	}

	// 초기값 브로드캐스트
	float MaxHealth = AttributeSet->GetMaxHealth();
	float CurrentHealth = AttributeSet->GetCurrentHealth();
	if (MaxHealth > 0.f)
	{
		OnCurrentHealthChanged.Broadcast(CurrentHealth / MaxHealth);
	}
	else
	{
		OnCurrentHealthChanged.Broadcast(0.f);
	}

	float MaxMana = AttributeSet->GetMaxMana();
	float CurrentMana = AttributeSet->GetCurrentMana();
	if (MaxMana > 0.f)
	{
		OnCurrentManaChanged.Broadcast(CurrentMana / MaxMana);
	}
	else
	{
		OnCurrentManaChanged.Broadcast(0.f);
	}

	float MaxStamina = AttributeSet->GetMaxStamina();
	float CurrentStamina = AttributeSet->GetCurrentStamina();
	if (MaxStamina > 0.f)
	{
		OnCurrentStaminaChanged.Broadcast(CurrentStamina / MaxStamina);
	}
	else
	{
		OnCurrentStaminaChanged.Broadcast(0.f);
	}

	// 재화 초기값 브로드캐스트 (정수)
	const int32 CurrentCurrency = FMath::RoundToInt(AttributeSet->GetCurrentCurrency());
	CachedCurrencyAmount = CurrentCurrency;
	OnCurrencyChanged.Broadcast(CurrentCurrency, 0); // 초기값이므로 DeltaAmount는 0
}

void UPawnUIComponent::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	const UCMAttributeSet* AttributeSet = CachedASC->GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		return;
	}

	const float MaxHealth = AttributeSet->GetMaxHealth();
	if (MaxHealth > 0.f)
	{
		OnCurrentHealthChanged.Broadcast(Data.NewValue / MaxHealth);
	}
	else
	{
		OnCurrentHealthChanged.Broadcast(0.f);
	}
}

void UPawnUIComponent::OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	const UCMAttributeSet* AttributeSet = CachedASC->GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		return;
	}

	const float CurrentHealth = AttributeSet->GetCurrentHealth();
	if (Data.NewValue > 0.f)
	{
		OnCurrentHealthChanged.Broadcast(CurrentHealth / Data.NewValue);
	}
	else
	{
		OnCurrentHealthChanged.Broadcast(0.f);
	}
}

void UPawnUIComponent::OnManaAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	const UCMAttributeSet* AttributeSet = CachedASC->GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		return;
	}

	const float MaxMana = AttributeSet->GetMaxMana();
	if (MaxMana > 0.f)
	{
		OnCurrentManaChanged.Broadcast(Data.NewValue / MaxMana);
	}
	else
	{
		OnCurrentManaChanged.Broadcast(0.f);
	}
}

void UPawnUIComponent::OnMaxManaAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	const UCMAttributeSet* AttributeSet = CachedASC->GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		return;
	}

	const float CurrentMana = AttributeSet->GetCurrentMana();
	if (Data.NewValue > 0.f)
	{
		OnCurrentManaChanged.Broadcast(CurrentMana / Data.NewValue);
	}
	else
	{
		OnCurrentManaChanged.Broadcast(0.f);
	}
}

void UPawnUIComponent::OnStaminaAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	const UCMAttributeSet* AttributeSet = CachedASC->GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		return;
	}

	const float MaxStamina = AttributeSet->GetMaxStamina();
	if (MaxStamina > 0.f)
	{
		OnCurrentStaminaChanged.Broadcast(Data.NewValue / MaxStamina);
	}
	else
	{
		OnCurrentStaminaChanged.Broadcast(0.f);
	}
}

void UPawnUIComponent::OnMaxStaminaAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	const UCMAttributeSet* AttributeSet = CachedASC->GetSet<UCMAttributeSet>();
	if (!AttributeSet)
	{
		return;
	}

	const float CurrentStamina = AttributeSet->GetCurrentStamina();
	if (Data.NewValue > 0.f)
	{
		OnCurrentStaminaChanged.Broadcast(CurrentStamina / Data.NewValue);
	}
	else
	{
		OnCurrentStaminaChanged.Broadcast(0.f);
	}
}

void UPawnUIComponent::AddAbilityIcon(const FCMPlayerAbilitySet& AbilitySet)
{
	// 서버에서만 호출되어야 함
	if (GetOwner()->HasAuthority())
	{
		// 중복 체크: 같은 SlotTag가 이미 있으면 업데이트 (슬롯 기준으로 중복 체크)
		FAbilityIconData* ExistingData = AbilityIcons.FindByPredicate([&AbilitySet](const FAbilityIconData& Data) {
			return Data.SlotTag == AbilitySet.SlotTag;
		});

		if (ExistingData)
		{
			ExistingData->InputTag = AbilitySet.InputTag;
			ExistingData->Icon = AbilitySet.SoftAbilityIconMaterial;
			ExistingData->CooldownTag = AbilitySet.CooldownTag;
		}
		else
		{
			AbilityIcons.Add(FAbilityIconData(AbilitySet.InputTag, AbilitySet.SlotTag, AbilitySet.SoftAbilityIconMaterial, AbilitySet.CooldownTag));
		}

		if (GetOwner()->GetNetMode() != NM_DedicatedServer)
		{
			OnRep_AbilityIcons();
		}
	}
}

void UPawnUIComponent::RemoveAbilityIcon(FGameplayTag SlotTag)
{
	if (GetOwner()->HasAuthority())
	{
		FAbilityIconData* ExistingData = AbilityIcons.FindByPredicate([&SlotTag](const FAbilityIconData& Data) {
			return Data.SlotTag == SlotTag;
		});

		if (ExistingData)
		{
			ExistingData->Icon = nullptr;
			ExistingData->InputTag = FGameplayTag::EmptyTag;
			ExistingData->CooldownTag = FGameplayTag::EmptyTag;

			if (GetOwner()->GetNetMode() != NM_DedicatedServer)
			{
				OnRep_AbilityIcons();
			}
		}
	}
}

void UPawnUIComponent::OnRep_AbilityIcons()
{
	// 클라이언트에서 리플리케이션 받았을 때 모든 아이콘 브로드캐스트
	for (const FAbilityIconData& IconData : AbilityIcons)
	{
		if (IconData.SlotTag.IsValid())
		{
			OnAbilityIconSlot.Broadcast(IconData);
		}
	}
}

void UPawnUIComponent::BroadcastInitAbilityIcons()
{
	// UI 초기화 시 호출: 저장된 모든 아이콘 브로드캐스트
	for (const FAbilityIconData& IconData : AbilityIcons)
	{
		if (IconData.SlotTag.IsValid())
		{
			OnAbilityIconSlot.Broadcast(IconData);
		}
	}
}

void UPawnUIComponent::StartAbilityCooldown(const FGameplayTag& InputTag, float Duration)
{
	// InputTag로 SlotTag 찾기
	const FAbilityIconData* IconData = AbilityIcons.FindByPredicate([&InputTag](const FAbilityIconData& Data) {
		return Data.InputTag == InputTag;
	});

	if (IconData && IconData->SlotTag.IsValid())
	{
		// SlotTag와 Duration을 UI에 브로드캐스트
		OnAbilityCooldownStarted.Broadcast(IconData->SlotTag, Duration);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UPawnUIComponent::StartAbilityCooldown - InputTag를 찾을 수 없음: %s"), *InputTag.ToString());
	}
}

void UPawnUIComponent::OnCurrencyAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 새 값을 정수로 변환
	const int32 NewAmount = FMath::RoundToInt(Data.NewValue);

	// 변경량 계산
	const int32 DeltaAmount = NewAmount - CachedCurrencyAmount;

	// 캐시 업데이트
	CachedCurrencyAmount = NewAmount;

	// 델리게이트 브로드캐스트 (UI 갱신용)
	if (OnCurrencyChanged.IsBound())
	{
		OnCurrencyChanged.Broadcast(NewAmount, DeltaAmount);
	}
}