// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CMGameplayTags.h"
#include "Character/CMCharacterBase.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Items/Weapons/CMWeaponBase.h"
#include "GameplayAbilitySystem/Effect/GE_DynamicCooldown.h"

void UCMGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (AbilityActivationPolicy == ECMAbilityActivationPolicy::OnGiven)
	{
		// 어빌리티가 아직 활성화되지 않았다면
		if (ActorInfo && !Spec.IsActive())
		{
			// 어빌리티 활성화를 시도 
			// 이 함수는 내부적으로 CanActivateAbility를 호출하므로 서버/클라 권한 체크가 자동으로 수행
			ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
		}
	}
}

UCMAbilitySystemComponent* UCMGameplayAbility::GetCMAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<UCMAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

ACMCharacterBase* UCMGameplayAbility::GetCMCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ACMCharacterBase>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

UPawnCombatComponent* UCMGameplayAbility::GetPawnCombatComponentFromActorInfo() const
{
	if (const ACMCharacterBase* CharacterBase = GetCMCharacterFromActorInfo())
	{
		return CharacterBase->GetPawnCombatComponent();
	}
	return nullptr;
}

float UCMGameplayAbility::GetCurrentAttackSpeed() const
{
	if (UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo())
	{
		// AttributeSet 값 읽기 (기본값 1.0f)
		bool bFound = false;
		const float Speed = ASC->GetGameplayAttributeValue(UCMAttributeSet::GetAttackSpeedAttribute(), bFound);
		return bFound ? Speed : 1.0f;
	}
	return 1.0f;
}

FGameplayEffectSpecHandle UCMGameplayAbility::MakeDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass, float InWeaponBaseDamage, float InGroggyDamage, FGameplayTag InAttackTypeTag, int32 InComboCount) const
{
	check(EffectClass);

	const UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return FGameplayEffectSpecHandle();
	}

	// 1. 컨텍스트 핸들 생성
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.SetAbility(this);                                                            // Ability 정보 추가 (누가 발동했는지) 
	ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());                              // 무기, 액터 등의 이펙트의 출처 객체
	ContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo()); // 이팩트를 유발한 주체 (자기자신)

	// 2. 스펙 핸들 생성
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), ContextHandle);

	// 3. SetByCaller 태그 설정
	if (SpecHandle.IsValid())
	{
		// 기본 데미지 설정
		SpecHandle.Data->SetSetByCallerMagnitude(CMGameplayTags::Shared_SetByCaller_BaseDamage, InWeaponBaseDamage);
		SpecHandle.Data->SetSetByCallerMagnitude(CMGameplayTags::Shared_SetByCaller_GroggyDamage, InGroggyDamage);

		// 콤보 횟수 설정
		if (InAttackTypeTag.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(InAttackTypeTag, InComboCount);
		}

		if (ASC->HasMatchingGameplayTag(CMGameplayTags::Player_Status_CanCounterAttack))
		{
			// 퍼펙트 패리 후 카운터 공격 보너스 (200% 데미지)
			SpecHandle.Data->SetSetByCallerMagnitude(CMGameplayTags::Shared_SetByCaller_CounterAttackBonus, CounterAttackDamageMultiplier);

			// 카운터 어택은 1회용이므로 태그 즉시 제거 (서버에서만)
			if (ASC->IsOwnerActorAuthoritative())
			{
				// const_cast는 위험한 cast이지만 이 경우는 ASC의 non-const 함수를 호출하기 위해 필요
				const_cast<UCMAbilitySystemComponent*>(ASC)->RemoveLooseGameplayTag(CMGameplayTags::Player_Status_CanCounterAttack);
			}
		}
	}

	return SpecHandle;
}

void UCMGameplayAbility::ExecuteWeaponHitGameplayCue(const AActor* HitActor, const FGameplayTag& InGameplayCueTag) const
{
	if (!HitActor)
	{
		return;
	}

	ACMCharacterBase* OwnerCharacter = GetCMCharacterFromActorInfo();
	if (!OwnerCharacter)
	{
		return;
	}

	UPawnCombatComponent* CombatComponent = GetPawnCombatComponentFromActorInfo();
	UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();

	if (!CombatComponent || !ASC)
	{
		return;
	}

	const FGameplayTag CueTagToUse = InGameplayCueTag.IsValid() ? InGameplayCueTag : GameplayCueTag;

	if (!CueTagToUse.IsValid())
	{
		return;
	}

	FGameplayCueParameters CueParams;
	CueParams.Instigator = OwnerCharacter;
	CueParams.EffectCauser = OwnerCharacter;
	CueParams.SourceObject = Cast<ACMWeaponBase>(CombatComponent->GetCharacterCurrentEquippedWeapon()); // Player/Enemy 모두 사용 가능한 PawnCombatComponent 함수 사용
	CueParams.TargetAttachComponent = HitActor->GetRootComponent();
	CueParams.Location = HitActor->GetActorLocation();
	CueParams.Normal = (OwnerCharacter->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal();

	ASC->ExecuteGameplayCue(CueTagToUse, CueParams);
}

void UCMGameplayAbility::ExecuteTargetHitGameplayCue(const AActor* HitActor, const FGameplayTag& InGameplayCueTag) const
{
	if (!HitActor || !InGameplayCueTag.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(HitActor));
	if (!TargetASC)
	{
		return;
	}

	ACMCharacterBase* OwnerCharacter = GetCMCharacterFromActorInfo();
	if (!OwnerCharacter)
	{
		return;
	}

	FGameplayCueParameters CueParams;
	CueParams.Instigator = OwnerCharacter;
	CueParams.EffectCauser = OwnerCharacter;
	CueParams.TargetAttachComponent = HitActor->GetRootComponent();
	CueParams.Location = HitActor->GetActorLocation();
	CueParams.Normal = (OwnerCharacter->GetActorLocation() - HitActor->GetActorLocation()).GetSafeNormal();

	TargetASC->ExecuteGameplayCue(InGameplayCueTag, CueParams);
}

const FGameplayTagContainer* UCMGameplayAbility::GetCooldownTags() const
{
	return CooldownIdentifierTags.Num() > 0 ? &CooldownIdentifierTags : nullptr;
}

UGameplayEffect* UCMGameplayAbility::GetCooldownGameplayEffect() const
{
	return UGE_DynamicCooldown::StaticClass()->GetDefaultObject<UGameplayEffect>();
}

void UCMGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();

	if (!ensure(CooldownGE))
	{
		return;
	}

	if (CooldownIdentifierTags.IsEmpty() || CooldownDurationSeconds <= 0.f)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CooldownGE->GetClass(), GetAbilityLevel());

	if (SpecHandle.IsValid())
	{
		// 적용할 태그
		SpecHandle.Data->DynamicGrantedTags.AppendTags(CooldownIdentifierTags);
		// SetByCaller에 지속시간을 담아 넘기기
		SpecHandle.Data->SetSetByCallerMagnitude(CMGameplayTags::SetByCaller_Cooldown_Duration, CooldownDurationSeconds);

		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
	else
	{
		ensureMsgf(false, TEXT("Failed to create Cooldown GameplayEffectSpec"));
	}
}