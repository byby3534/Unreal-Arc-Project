// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility_Melee.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CMGameplayTags.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"

UCMEnemyGameplayAbility_Melee::UCMEnemyGameplayAbility_Melee()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer Tags;
	Tags.AddTag(CMGameplayTags::Enemy_Ability_Attack_Melee);
	SetAssetTags(Tags);

	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_HitReact);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Groggy);
}

void UCMEnemyGameplayAbility_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo());
	if (!EnemyCharacter || !AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 타겟 찾기
	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());

	// TriggerData에 없다면 블랙보드 확인
	if (!TargetActor)
	{
		TargetActor = EnemyCharacter->StateTreeTargetActor;
	}

	// 모션 워핑 설정 (타겟이 있다면 바라보게 설정)
	if (TargetActor)
	{
		if (UMotionWarpingComponent* MotionWarping = EnemyCharacter->FindComponentByClass<UMotionWarpingComponent>())
		{
			MotionWarping->AddOrUpdateWarpTargetFromComponent(
			WarpTargetName,
			TargetActor->GetRootComponent(),
			NAME_None,
			true,
			FVector::ZeroVector
			);
		}
	}

	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// 히트 이벤트 대기
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		CMGameplayTags::Shared_Event_MeleeHit
		);
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackHit);
	WaitEventTask->ReadyForActivation();
}

void UCMEnemyGameplayAbility_Melee::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMEnemyGameplayAbility_Melee::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMEnemyGameplayAbility_Melee::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMEnemyGameplayAbility_Melee::OnAttackHit(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());
	if (!HitActor || !DamageEffectClass)
	{
		return;
	}

	// 데미지 Spec 생성

	FGameplayEffectSpecHandle SpecHandle = MakeDamageEffectSpecHandle(
		DamageEffectClass,
		SkillBaseDamage * DamageMultiplier,
		0,
		FGameplayTag(),
		0
		);

	if (SpecHandle.IsValid())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
		{
			UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
}