// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerGameplayAbility_TargetLock.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerGameplayAbility_TargetLock : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerGameplayAbility_TargetLock();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetLockOnState(bool bShouldLockOn, AActor* NewTarget);

	void ApplyLockOnState(bool bShouldLockOn, AActor* NewTarget) const;
};