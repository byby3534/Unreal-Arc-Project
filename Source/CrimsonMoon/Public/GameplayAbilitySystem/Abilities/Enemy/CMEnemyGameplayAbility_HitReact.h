// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility.h"
#include "CMEnemyGameplayAbility_HitReact.generated.h"

/**
 * 적 AI 피격 어빌리티
 * - Shared.Event.HitReact 이벤트로 트리거
 * - 실행 시 이동 즉시 정지
 * - 몽타주 재생 중 Shared.Status.HitReact 태그 보유
 */
UCLASS()
class CRIMSONMOON_API UCMEnemyGameplayAbility_HitReact : public UCMEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UCMEnemyGameplayAbility_HitReact();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 피격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UFUNCTION()
	void OnMontageEnded();
};