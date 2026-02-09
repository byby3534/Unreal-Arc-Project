// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGA_PhaseTransition.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayTags/CMGameplayTags_Enemy.h"
#include "GameplayTags/CMGameplayTags_Shared.h"

UCMEnemyGA_PhaseTransition::UCMEnemyGA_PhaseTransition()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer Tags;
    Tags.AddTag(CMGameplayTags::Enemy_Ability_PhaseTransition);
    SetAssetTags(Tags);

	ActivationOwnedTags.AddTag(CMGameplayTags::Shared_Status_SuperArmor);

	CancelAbilitiesWithTag.AddTag(CMGameplayTags::Enemy_Ability_Attack);

}

void UCMEnemyGA_PhaseTransition::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (RoarMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, RoarMontage
			);
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주 없으면 즉시 종료 (버프만 적용)
		OnMontageEnded();
	}
}

void UCMEnemyGA_PhaseTransition::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMEnemyGA_PhaseTransition::OnMontageEnded()
{
	if (HasAuthority(&CurrentActivationInfo) && BuffEffectClass)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			const UGameplayEffect* EffectCDO = BuffEffectClass->GetDefaultObject<UGameplayEffect>();

			FGameplayTagContainer EffectGrantedTags = EffectCDO->GetGrantedTags();

			bool bAlreadyInPhase = ASC->HasAnyMatchingGameplayTags(EffectGrantedTags);

			if (!bAlreadyInPhase)
			{
				FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
				ContextHandle.AddSourceObject(this);

				// 버프 적용
				ASC->ApplyGameplayEffectToSelf(
					EffectCDO,
					1.0f,
					ContextHandle
					);

				UE_LOG(LogTemp, Log, TEXT("페이즈 전환! 적용된 태그: %s"), *EffectGrantedTags.ToString());
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}