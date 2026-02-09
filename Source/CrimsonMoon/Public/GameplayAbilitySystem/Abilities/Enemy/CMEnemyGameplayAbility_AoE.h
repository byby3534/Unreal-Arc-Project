// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility.h"
#include "CMEnemyGameplayAbility_AoE.generated.h"

/**
 * 광역 공격 어빌리티
 * AoE 컴포넌트 사용
 */
UCLASS()
class CRIMSONMOON_API UCMEnemyGameplayAbility_AoE : public UCMEnemyGameplayAbility
{
	GENERATED_BODY()
public:

	UCMEnemyGameplayAbility_AoE();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	
	/** 공격 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 적용할 데미지 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/* 모션 워핑 타겟 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionWarping")
	FName WarpTargetName = FName("FacingTarget");

	/** 기본 데미지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float BaseDamage = 10.0f;

	/** 공격 반경 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRadius = 300.0f;

	/** AoE 발생 기준 소켓 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName AoESocketName = NAME_None;

	/** AoE 발생 위치 오프셋 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FVector AoELocationOffset = FVector::ZeroVector;

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnMontageCancelled();

	/** 몽타주에서 발생한 이벤트를 받아 광역 데미지 실행 */
	UFUNCTION()
	void OnAoETriggered(FGameplayEventData Payload);
};
