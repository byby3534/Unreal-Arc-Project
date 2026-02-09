// Copyright CrimsonMoon Team. All Rights Reserved.

#include "GameplayAbilitySystem/Abilities/Player/Blade/CMPlayerAbility_SkillF_Blade.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CMGameplayTags.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

UCMPlayerAbility_SkillF_Blade::UCMPlayerAbility_SkillF_Blade()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Skill_Blade_F);
	SetAssetTags(TagsToAdd);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UCMPlayerAbility_SkillF_Blade::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	float AttackSpeed = GetCurrentAttackSpeed();

	if (!ChargeStartMontage)
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

	// 차지 시작
	StartCharging();
}

void UCMPlayerAbility_SkillF_Blade::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 차징 태그 정리
	if (bIsCharging)
	{
		if (UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(CMGameplayTags::Player_Status_Charging);
		}
	}

	// 상태 초기화
	bIsCharging = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMPlayerAbility_SkillF_Blade::StartCharging()
{
	bIsCharging = true;

	// 차징 시작 시 태그 부여
	if (UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->AddLooseGameplayTag(CMGameplayTags::Player_Status_Charging);
	}

	// 몽타주 유효성 검사
	if (!ChargeStartMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ChargeStartMontage,
		1.0f,
		NAME_None,
		false,
		1.0f
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::StartChargeLoop);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::StartChargeLoop);

	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UCMPlayerAbility_SkillF_Blade::StartChargeLoop()
{
	if (!ChargeLoopMontage)
	{
		ExecuteSlash();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ChargeLoopMontage,
		1.0f,
		NAME_None,
		false
		);

	MontageTask->ReadyForActivation();

	UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, ChargeTime);

	WaitTask->OnFinish.AddDynamic(this, &ThisClass::ExecuteSlash);
	WaitTask->ReadyForActivation();
}

void UCMPlayerAbility_SkillF_Blade::ExecuteSlash()
{
	float AttackSpeed = GetCurrentAttackSpeed();
	if (!SlashMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bIsCharging = false;

	// 차징 태그 제거
	if (UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CMGameplayTags::Player_Status_Charging);
	}

	const bool bHasAuthority = HasAuthority(&CurrentActivationInfo);

	// [서버 전용] 데미지 이벤트 대기
	if (bHasAuthority)
	{
		UAbilityTask_WaitGameplayEvent* WaitHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CMGameplayTags::Shared_Event_MeleeHit);
		WaitHitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitTarget);
		WaitHitTask->ReadyForActivation();
	}

	// [서버 & 클라이언트] 광역 베기 몽타주 재생 (AbilityTask 사용 - 자동 복제)
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SlashMontage, AttackSpeed);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnSlashMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnSlashMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UCMPlayerAbility_SkillF_Blade::OnChargeStartMontageCompleted()
{
	// 차징 시작 완료 - 차징 루프 시작
	StartChargeLoop();
}

void UCMPlayerAbility_SkillF_Blade::OnSlashMontageCompleted()
{
	// 광역 베기 몽타주가 정상 종료됨
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_SkillF_Blade::OnMontageCancelled()
{
	// 몽타주가 취소됨 (피격 등)
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMPlayerAbility_SkillF_Blade::OnHitTarget(FGameplayEventData Payload)
{
	// [서버 전용] AnimNotify에서 발생한 MeleeHit 이벤트 처리
	// 광역 공격은 무기의 BoxComponent/SphereComponent + AN_ToggleCollision으로 처리됨

	const AActor* HitActor = Payload.Target.Get();
	if (!HitActor || !DamageEffect)
	{
		return;
	}

	// 캐릭터 및 컴포넌트 가져오기
	ACMPlayerCharacterBase* OwnerCharacter = Cast<ACMPlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter)
	{
		return;
	}

	UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();
	if (!PlayerCombatComponent)
	{
		return;
	}

	// 무기 기본 데미지 가져오기
	const float WeaponBaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel());
	const float BaseGroggyDamage = PlayerCombatComponent->GetPlayerCurrentWeaponHeavyGroggyDamage(GetAbilityLevel());

	// DamageEffect 스펙 생성 (헤비 공격으로 처리)
	FGameplayEffectSpecHandle SpecHandle = MakeDamageEffectSpecHandle(
		DamageEffect,
		WeaponBaseDamage,
		BaseGroggyDamage,
		CMGameplayTags::Player_SetByCaller_AttackType_Heavy,
		1 // 콤보 카운트 1
		);

	ExecuteWeaponHitGameplayCue(HitActor, GameplayCueTag);

	if (SpecHandle.IsValid())
	{
		// 타겟의 ASC 가져오기
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(HitActor));
		if (TargetASC)
		{
			// 타겟에 데미지 적용
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get(), GetAbilitySystemComponentFromActorInfo()->ScopedPredictionKey);

			// 타겟의 ASC에서 Hit Sound Gameplay Cue 실행
			ExecuteTargetHitGameplayCue(HitActor, CMGameplayTags::GameplayCue_Sounds_MeleeHit_Blade);
		}
	}
}