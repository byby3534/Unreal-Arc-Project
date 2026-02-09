// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility.h"
#include "CMEnemyGameplayAbility_Melee.generated.h"

/**
 * [적 AI 전용 근접 공격]
 * State Tree에서 이 어빌리티를 태그로 호출
 * 활성화되면 Motion Warping으로 타겟을 정렬하고 몽타주를 재생합니다.
 */
UCLASS()
class CRIMSONMOON_API UCMEnemyGameplayAbility_Melee : public UCMEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UCMEnemyGameplayAbility_Melee();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/* 재생할 공격 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	/* 데미지 적용 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/* 모션 워핑 타겟 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionWarping")
	FName WarpTargetName = FName("FacingTarget");

	/* 스킬의 기본 데미지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float SkillBaseDamage = 10.0f;

	/* 데미지 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float DamageMultiplier = 1.0f;

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnMontageCancelled();

	/* AnimNotify 수신 시 호출 */
	UFUNCTION()
	void OnAttackHit(FGameplayEventData Payload);
};