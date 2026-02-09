// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility.h"
#include "CMEnemyGA_PhaseTransition.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMEnemyGA_PhaseTransition : public UCMEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UCMEnemyGA_PhaseTransition();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 포효 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Phase")
	TObjectPtr<UAnimMontage> RoarMontage;

	/** 페이즈 적용 버프 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Phase")
	TSubclassOf<UGameplayEffect> BuffEffectClass;

	UFUNCTION()
	void OnMontageEnded();
};