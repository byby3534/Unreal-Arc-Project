// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"
#include "CMPlayerAbility_UnequipConsumable.generated.h"

/**
 * 소모품(물약, 수류탄 등)을 손에서 제거하는 어빌리티
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_UnequipConsumable : public UCMGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_UnequipConsumable();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// 찾을 컴포넌트 태그 이름
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FName TargetMeshTagName = FName("AttachedConsumableMesh");
};
