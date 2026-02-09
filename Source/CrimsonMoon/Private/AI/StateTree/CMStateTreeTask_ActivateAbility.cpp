// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/StateTree/CMStateTreeTask_ActivateAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FCMStateTreeTask_ActivateAbility::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 태그로 어빌리티 활성화 시도
	if (AbilityTagToActivate.IsValid())
	{
		if (ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTagToActivate)))
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}
	
	return EStateTreeRunStatus::Failed;
}