// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/CMAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Components/UI/UPawnUIComponent.h"
#include "GameplayTags/CMGameplayTags_Shared.h"

UCMAttributeSet::UCMAttributeSet()
{
	InitCurrentHealth(1.f);
	InitMaxHealth(1.f);
	InitCurrentMana(1.f);
	InitMaxMana(1.f);
	InitMaxStamina(1.f);
	InitCurrentStamina(1.f);
	InitBaseDamage(1.f);
	InitDamageTaken(0.f);
	InitDefensePower(1.f);
	InitAttackPower(1.f);
	InitCurrentGroggy(1.f);
	InitMaxGroggy(1.f);
	InitGroggyDamageTaken(0.f);
	InitAttackSpeed(1.f);
	InitMoveSpeed(600.f);

	// 재화 초기화 (정수처럼 운용)
	InitCurrentCurrency(0.f);
	InitMaxCurrency(0.f); // 0 = 무제한
}

void UCMAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, CurrentHealth, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, MaxHealth, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, CurrentMana, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, MaxMana, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, CurrentStamina, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, MaxStamina, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, BaseDamage, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, DamageTaken, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, AttackPower, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, DefensePower, COND_None, REPNOTIFY_OnChanged)
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, CurrentGroggy, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, MaxGroggy, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, GroggyDamageTaken, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, AttackSpeed, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, MoveSpeed, COND_None, REPNOTIFY_OnChanged);

	#pragma region Currency Attribute
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, CurrentCurrency, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UCMAttributeSet, MaxCurrency, COND_None, REPNOTIFY_OnChanged);
	#pragma endregion
}

void UCMAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 현재값은 0과 최대값 사이로 강제함
	if (Attribute == GetCurrentHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetCurrentManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetCurrentStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetCurrentGroggyAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxGroggy());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	// 재화는 정수처럼 운용: 항상 반올림하고, 0 이상 유지
	// MaxCurrency가 0보다 크면 상한선 적용, 0이면 무제한
	else if (Attribute == GetCurrentCurrencyAttribute())
	{
		NewValue = FMath::RoundToFloat(NewValue);
		NewValue = FMath::Max(NewValue, 0.f);
		const float MaxCurrencyValue = GetMaxCurrency();
		if (MaxCurrencyValue > 0.f)
		{
			NewValue = FMath::Min(NewValue, MaxCurrencyValue);
		}
	}
	else if (Attribute == GetMaxCurrencyAttribute())
	{
		NewValue = FMath::RoundToFloat(NewValue);
		NewValue = FMath::Max(NewValue, 0.f); // 0 = 무제한
	}
}

void UCMAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		SetCurrentHealth(FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentManaAttribute())
	{
		SetCurrentMana(FMath::Clamp(GetCurrentMana(), 0.f, GetMaxMana()));
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentStaminaAttribute())
	{
		SetCurrentStamina(FMath::Clamp(GetCurrentStamina(), 0.f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentGroggyAttribute())
	{
		SetCurrentGroggy(FMath::Clamp(GetCurrentGroggy(), 0.f, GetMaxGroggy()));
	}
	else if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
	{
		SetMoveSpeed(FMath::Max(GetMoveSpeed(), 0.f));
	}

	// 체력 소모 로직
	if (Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
	{
		HandleDamageAndTriggerHitReact(Data);
	}

	// 그로기 피해 누적 로직
	if (Data.EvaluatedData.Attribute == GetGroggyDamageTakenAttribute())
	{
		HandleGroggyDamage(Data);
	}
}

void UCMAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, CurrentHealth, OldValue);
}

void UCMAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, MaxHealth, OldValue);
}

void UCMAttributeSet::OnRep_CurrentMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, CurrentMana, OldValue);
}

void UCMAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, MaxMana, OldValue);
}

void UCMAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, MaxStamina, OldValue);
}

void UCMAttributeSet::OnRep_CurrentStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, CurrentStamina, OldValue);
}

void UCMAttributeSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, BaseDamage, OldValue);
}

void UCMAttributeSet::OnRep_DamageTaken(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, DamageTaken, OldValue);
}

void UCMAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, AttackPower, OldValue);
}

void UCMAttributeSet::OnRep_DefensePower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, DefensePower, OldValue);
}

void UCMAttributeSet::OnRep_CurrentGroggy(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, CurrentGroggy, OldValue);
}

void UCMAttributeSet::OnRep_MaxGroggy(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, MaxGroggy, OldValue);
}

void UCMAttributeSet::OnRep_GroggyDamageTaken(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, GroggyDamageTaken, OldValue);
}

void UCMAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, AttackSpeed, OldValue);
}

void UCMAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, MoveSpeed, OldValue);
}

void UCMAttributeSet::OnRep_CurrentCurrency(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, CurrentCurrency, OldValue);
}

void UCMAttributeSet::OnRep_MaxCurrency(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCMAttributeSet, MaxCurrency, OldValue);
}

bool UCMAttributeSet::TryBlockOrParry(UAbilitySystemComponent* TargetASC, UAbilitySystemComponent* SourceASC, AActor* Attacker, AActor* Defender, bool& bOutWasBlockAttempted, bool& bOutBlockSucceeded) const
{
	bOutWasBlockAttempted = false;
	bOutBlockSucceeded = false; // 수동으로 데미지 감소 (25% 감소)

	if (!TargetASC || !Attacker || !Defender)
	{
		return false;
	}

	// Unparryable 체크
	bool bIsUnparryable = false;
	if (SourceASC)
	{
		bIsUnparryable = SourceASC->HasMatchingGameplayTag(CMGameplayTags::Shared_Attack_Unparryable);
	}

	// 블록/패링 판정
	const bool bIsPerfectParry = TargetASC->HasMatchingGameplayTag(CMGameplayTags::Player_Status_PerfectParryWindow);
	const bool bIsBlocking = TargetASC->HasMatchingGameplayTag(CMGameplayTags::Player_Status_Blocking);

	// 블록을 시도했는지 기록 (방향 무관)
	bOutWasBlockAttempted = (bIsPerfectParry || bIsBlocking) && !bIsUnparryable;

	// 방향 체크
	const bool bValidDirection = UCMFunctionLibrary::IsValidBlock(Attacker, Defender);

	if (bOutWasBlockAttempted && bValidDirection)
	{
		// 공통 이벤트 데이터 생성
		FGameplayEventData Payload;
		Payload.Target = Defender;
		Payload.Instigator = Attacker;

		if (bIsPerfectParry)
		{
			// 퍼펙트 패링 성공 이벤트 (카운터 어택 태그는 CounterAttackWindowEffect에서 부여)
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Defender, CMGameplayTags::Player_Event_PerfectParrySuccess, Payload);
		}
		else
		{
			// 일반 블록 성공 (퍼펙트 패링 아님)
			bOutBlockSucceeded = true;
		}

		// 블록 성공 이벤트 (퍼펙트 패링이든 일반 블록이든 모두 발생)
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Defender, CMGameplayTags::Player_Event_SuccessfulBlock, Payload);

		// 퍼펙트 패링이면 true (데미지 완전 무효화), 일반 블록이면 false (데미지 감소)
		return bIsPerfectParry;
	}

	return false;
}

void UCMAttributeSet::HandleGroggyDamage(const FGameplayEffectModCallbackData& Data)
{
	const float GroggyDamage = GetGroggyDamageTaken();
	SetGroggyDamageTaken(0.f);

	UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();

	// 이미 그로기 상태라면 누적하지 않음
	if (TargetASC && TargetASC->HasMatchingGameplayTag(CMGameplayTags::Shared_Status_Groggy))
	{
		return;
	}

	// 무적or슈퍼아머 태그 보유시 그로기 데미지 무효화
	static FGameplayTagContainer GroggyImmunityTags;
	if (GroggyImmunityTags.IsEmpty())
	{
		GroggyImmunityTags.AddTag(CMGameplayTags::Shared_Status_Invincible);
		GroggyImmunityTags.AddTag(CMGameplayTags::Shared_Status_SuperArmor);
	}

	if (TargetASC && TargetASC->HasAnyMatchingGameplayTags(GroggyImmunityTags))
	{
		return;
	}

	const float OldGroggy = GetCurrentGroggy();
	const float NewGroggy = OldGroggy + GroggyDamage;
	SetCurrentGroggy(NewGroggy);

	if (NewGroggy >= GetMaxGroggy())
	{
		// GA_Groggy를 통해 그로기 상태 처리
		FGameplayEventData Payload;
		Payload.EventTag = CMGameplayTags::Shared_Event_GroggyTriggered;
		Payload.Target = Data.Target.GetAvatarActor();

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Data.Target.GetAvatarActor(),
			CMGameplayTags::Shared_Event_GroggyTriggered,
			Payload
			);
	}
}

void UCMAttributeSet::HandleDamageAndTriggerHitReact(const FGameplayEffectModCallbackData& Data)
{
	UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();

	// 무적 태그 보유시 데미지 무효화
	if (TargetASC && TargetASC->HasMatchingGameplayTag(CMGameplayTags::Shared_Status_Invincible))
	{
		SetDamageTaken(0.f);
		return;
	}

	constexpr float DiscountDamage = 0.25f;
	const float PreviousHealth = GetCurrentHealth();
	const float DamageDone = GetDamageTaken();
	SetDamageTaken(0.f);

	UAbilitySystemComponent* SourceASC = Data.EffectSpec.GetEffectContext().GetInstigatorAbilitySystemComponent();
	AActor* Attacker = Data.EffectSpec.GetEffectContext().GetInstigator();
	AActor* Defender = Data.Target.GetAvatarActor();

	// 블록/패링 시도
	bool bWasBlockAttempted = false;
	bool bBlockSucceeded = false;
	const bool bWasParried = TryBlockOrParry(TargetASC, SourceASC, Attacker, Defender, bWasBlockAttempted, bBlockSucceeded);

	if (!bWasParried) // 패링에 성공하지 못했을 때 데미지를 적용
	{
		float FinalDamage = DamageDone;

		// 일반 블록 성공 시 데미지 감소 (각도 검증 통과)
		if (bBlockSucceeded)
		{
			// 수동으로 데미지 감소 (25% 감소)
			FinalDamage *= DiscountDamage;
		}

		const float NewCurrentHealth = PreviousHealth - FinalDamage;
		SetCurrentHealth(NewCurrentHealth);
	}

	// 사망 판정
	if (GetCurrentHealth() <= 0.f)
	{
		UCMFunctionLibrary::AddGameplayTagToActorIfNone(
			Data.Target.GetAvatarActor(),
			CMGameplayTags::Shared_Status_Dead
			);

		// 사망 이벤트 전송 (Death Drop Ability 트리거용)
		FGameplayEventData Payload;
		Payload.EventTag = CMGameplayTags::Shared_Event_Death;
		Payload.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
		Payload.Target = Data.Target.GetAvatarActor();

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Data.Target.GetAvatarActor(),
			CMGameplayTags::Shared_Event_Death,
			Payload
			);
	}
	else if (!bWasParried)
	{
		static FGameplayTagContainer HitReactImmunityTags;
		if (HitReactImmunityTags.IsEmpty())
		{
			HitReactImmunityTags.AddTag(CMGameplayTags::Shared_Status_Groggy);
			HitReactImmunityTags.AddTag(CMGameplayTags::Shared_Status_SuperArmor);
			HitReactImmunityTags.AddTag(CMGameplayTags::Shared_Status_Invincible);
		}

		if (TargetASC && !TargetASC->HasAnyMatchingGameplayTags(HitReactImmunityTags))
		{
			FGameplayEventData Payload;
			Payload.EventTag = CMGameplayTags::Shared_Event_HitReact;
			Payload.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
			Payload.Target = Data.Target.GetAvatarActor();
			Payload.EventMagnitude = DamageDone;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				Data.Target.GetAvatarActor(),
				CMGameplayTags::Shared_Event_HitReact,
				Payload
				);
		}
	}
}