// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_HitReact.h"

#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UCMPlayerAbility_HitReact::UCMPlayerAbility_HitReact()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Shared_Ability_HitReact);
	SetAssetTags(TagsToAdd);

	CancelAbilitiesWithTag.AddTag(CMGameplayTags::Player_Ability_Base_Attack);
	// CancelAbilitiesWithTag.AddTag(CMGameplayTags::Player_Ability_Block); // ← Block은 취소하지 않음 (패리 시스템)
	CancelAbilitiesWithTag.AddTag(CMGameplayTags::Player_Ability_EquipWeapon);
	CancelAbilitiesWithTag.AddTag(CMGameplayTags::Player_Ability_UnEquipWeapon);

	BlockAbilitiesWithTag.AddTag(CMGameplayTags::Player_Ability);

	// 블록 중일 때는 HitReact가 활성화되지 않도록
	ActivationBlockedTags.AddTag(CMGameplayTags::Player_Status_Blocking);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// 트리거 등록
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = CMGameplayTags::Shared_Event_HitReact;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UCMPlayerAbility_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AActor* InstigatorActor = TriggerEventData ? TriggerEventData->Instigator.Get() : nullptr;
	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	UAnimMontage* MontageToPlay = FrontHitReactMontage;

	if (InstigatorActor && AvatarActor)
	{
		float AngleDifference = 0.f;
		// 공격자와의 각도를 계산하여 방향 태그(Front, Left, Right, Back)를 반환받음
		const FGameplayTag HitDirectionTag = UCMFunctionLibrary::ComputeHitReactDirectionTag(InstigatorActor, AvatarActor, AngleDifference);

		if (HitDirectionTag.MatchesTagExact(CMGameplayTags::Shared_Status_HitReact_Left))
		{
			MontageToPlay = LeftHitReactMontage;
		}
		else if (HitDirectionTag.MatchesTagExact(CMGameplayTags::Shared_Status_HitReact_Right))
		{
			MontageToPlay = RightHitReactMontage;
		}
		else if (HitDirectionTag.MatchesTagExact(CMGameplayTags::Shared_Status_HitReact_Back))
		{
			MontageToPlay = BackHitReactMontage;
		}
	}

	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageToPlay,
			1.0f,
			NAME_None,
			false,
			1.0f
			);

		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

		MontageTask->ReadyForActivation();
	}
	else
	{
		// 재생할 몽타주가 없으면 즉시 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UCMPlayerAbility_HitReact::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_HitReact::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}