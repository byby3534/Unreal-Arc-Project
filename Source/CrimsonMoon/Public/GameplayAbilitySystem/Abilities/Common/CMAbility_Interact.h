// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"
#include "CMAbility_Interact.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMAbility_Interact : public UCMGameplayAbility
{
	GENERATED_BODY()

public:
	UCMAbility_Interact();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// 트레이스 설정 (CMLineTraceComponent에 넘겨줄 값)
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionRange = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};
