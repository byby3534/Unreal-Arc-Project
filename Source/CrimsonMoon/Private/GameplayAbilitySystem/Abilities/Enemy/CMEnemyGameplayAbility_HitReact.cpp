// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility_HitReact.h"

#include "CMGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

UCMEnemyGameplayAbility_HitReact::UCMEnemyGameplayAbility_HitReact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationOwnedTags.AddTag(CMGameplayTags::Shared_Status_HitReact);

	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Groggy);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_SuperArmor);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Invincible);

	// 공격 어빌리티들 중단
	CancelAbilitiesWithTag.AddTag(CMGameplayTags::Enemy_Ability_Attack);

	// 트리거 등록
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = CMGameplayTags::Shared_Event_HitReact;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UCMEnemyGameplayAbility_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!HitReactMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo());
	if (!EnemyCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 이동 정지
	if (UCharacterMovementComponent* MoveComp = EnemyCharacter->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(EMovementMode::MOVE_None);
	}

	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		HitReactMontage
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UCMEnemyGameplayAbility_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 다시 이동할 수 있게 복구
	if (ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo()))
	{
		if (UCharacterMovementComponent* MoveComp = EnemyCharacter->GetCharacterMovement())
		{
			MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMEnemyGameplayAbility_HitReact::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}