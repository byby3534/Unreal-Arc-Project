// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"
#include "CMAbility_MonsterDeathDrop.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMAbility_MonsterDeathDrop : public UCMGameplayAbility
{
	GENERATED_BODY()

public:
	UCMAbility_MonsterDeathDrop();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
