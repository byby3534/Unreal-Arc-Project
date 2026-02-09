// Copyright CrimsonMoon Team. All Rights Reserved.

#include "GameplayAbilitySystem/Abilities/Player/Blade/CMPlayerAbility_ComboAttack_Base.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "CMGameplayTags.h"
#include "Items/Weapons/CMWeaponBase.h"

UCMPlayerAbility_ComboAttack_Base::UCMPlayerAbility_ComboAttack_Base()
{
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);
	ActivationBlockedTags.AddTag(CMGameplayTags::Player_Status_Charging);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UCMPlayerAbility_ComboAttack_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 공격속도
	float AttackSpeed = GetCurrentAttackSpeed();

	if (AttackMontages.IsEmpty())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 커밋에 실패하면 (예: 스태미나 부족) 어빌리티를 즉시 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const bool bHasAuthority = HasAuthority(&ActivationInfo);

	UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();
	bool bIsCounterAttack = ASC && ASC->HasMatchingGameplayTag(CMGameplayTags::Player_Status_CanCounterAttack);

	UAnimMontage* MontageToPlay = nullptr;

	// [서버 & 클라이언트] 콤보 횟수 확인 (클라이언트는 복제된 값으로 예측)
	if (bIsCounterAttack && CounterAttackMontage)
	{
		// 카운터 어택 몽타주 재생
		MontageToPlay = CounterAttackMontage;
		CurrentComboCount = 0;
		if (bHasAuthority)
		{
			// 클라이언트의 예측이 틀렸을 수도 있으니 0으로 동기화
			Client_SyncComboCount(CurrentComboCount);
		}
	}
	else
	{
		// [기존 콤보 로직]
		if (AttackMontages.IsEmpty())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		// 콤보 카운트가 몽타주 배열의 끝에 도달했다면 0으로 리셋
		if (CurrentComboCount >= AttackMontages.Num())
		{
			CurrentComboCount = 0;
			if (bHasAuthority)
			{
				Client_SyncComboCount(CurrentComboCount);
			}
		}
		MontageToPlay = AttackMontages[CurrentComboCount];
	}

	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// [서버 전용] 데미지 이벤트 대기
	if (bHasAuthority)
	{
		UAbilityTask_WaitGameplayEvent* WaitHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CMGameplayTags::Shared_Event_MeleeHit);
		WaitHitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitTarget);
		WaitHitTask->ReadyForActivation();
	}

	// [서버 & 클라이언트] 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		AttackSpeed,
		NAME_None,
		false
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// [서버 전용] 콤보 횟수 증가 (카운터 어택이 아닐 때만)
	if (bHasAuthority && !bIsCounterAttack)
	{
		CurrentComboCount++;
		Client_SyncComboCount(CurrentComboCount);
	}
}

void UCMPlayerAbility_ComboAttack_Base::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 어빌리티가 (피격, 닷지 등으로) '취소'되었다면
	if (bWasCancelled)
	{
		// [서버 전용] 서버에서만 콤보 취소 처리
		if (HasAuthority(&ActivationInfo))
		{
			HandleComboCancelled();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UCMPlayerAbility_ComboAttack_Base::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UCMPlayerAbility_ComboAttack_Base::HandleComboComplete()
{
	// 기본 구현: 아무것도 안함
	// 자식 클래스에서 오버라이드하여 타이머 시작 등 구현
}

void UCMPlayerAbility_ComboAttack_Base::HandleComboCancelled()
{
	// 기본 구현: 즉시 리셋
	ResetComboCount();
}

void UCMPlayerAbility_ComboAttack_Base::ResetComboCount()
{
	// 이 함수는 서버에서만 호출되어야 함
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	CurrentComboCount = 0;
	Client_SyncComboCount(CurrentComboCount);
}

void UCMPlayerAbility_ComboAttack_Base::OnMontageEnded()
{
	// [중요] 'InstancedPerActor' 어빌리티는 EndAbility가 호출되면 GC 대상이 됨
	// EndAbility를 먼저 호출하여 객체를 비활성 상태로 만든 뒤,
	// 자식 클래스의 HandleComboComplete가 타이머를 설정하면 GC가 객체를 보호함
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

	// [서버 전용] 몽타주가 '정상 종료'되었다면, 서버에서만 콤보 완료 처리
	if (HasAuthority(&CurrentActivationInfo))
	{
		HandleComboComplete();
	}
}

void UCMPlayerAbility_ComboAttack_Base::OnMontageCancelled()
{
	// 몽타주가 '취소' (중단)되었다면, 어빌리티를 취소 상태로 종료
	// EndAbility(bWasCancelled=true)가 호출되어 HandleComboCancelled 실행
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMPlayerAbility_ComboAttack_Base::OnHitTarget(FGameplayEventData Payload)
{
	// [서버] 커스텀 계산기로 데미지 적용
	// 이 함수는 서버의 'WaitHitTask'에 의해서만 호출되므로, HasAuthority() 검사가 필요 없음
	const AActor* HitActor = Payload.Target.Get();
	ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();

	if (!OwnerCharacter || !HitActor || !DamageEffect)
	{
		return;
	}

	UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();
	UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();
	if (!PlayerCombatComponent || !ASC)
	{
		return;
	}

	// 1. 무기의 기본 데미지 가져오기
	const float BaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel());
	const int32 ComboCount = CurrentComboCount;
	const FGameplayTag AttackType = ComboAttackTypeTag;

	float GroggyDamage = 0.f;

	if (ASC->HasMatchingGameplayTag(CMGameplayTags::Player_Status_CanCounterAttack))
	{
		// 카운터 어택
		GroggyDamage = PlayerCombatComponent->GetPlayerCurrentWeaponCounterGroggyDamage(GetAbilityLevel());
	}
	else if (AttackType.MatchesTagExact(CMGameplayTags::Player_SetByCaller_AttackType_Heavy))
	{
		// 강공격
		GroggyDamage = PlayerCombatComponent->GetPlayerCurrentWeaponHeavyGroggyDamage(GetAbilityLevel());
	}

	FGameplayEffectSpecHandle SpecHandle = MakeDamageEffectSpecHandle(DamageEffect, BaseDamage, GroggyDamage, ComboAttackTypeTag, ComboCount);

	// Slash VFX 재생
	ExecuteWeaponHitGameplayCue(HitActor, GameplayCueTag);

	if (SpecHandle.IsValid())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(HitActor)))
		{
			// 데미지 적용
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get(), GetAbilitySystemComponentFromActorInfo()->ScopedPredictionKey);

			// 타겟의 ASC에서 Hit Sound Gameplay Cue 실행
			ExecuteTargetHitGameplayCue(HitActor, CMGameplayTags::GameplayCue_Sounds_MeleeHit_Blade);
		}
	}
}

void UCMPlayerAbility_ComboAttack_Base::Client_SyncComboCount_Implementation(int32 NewComboCount)
{
	// [클라이언트] 서버로부터 콤보 카운트 수신
	CurrentComboCount = NewComboCount;
}