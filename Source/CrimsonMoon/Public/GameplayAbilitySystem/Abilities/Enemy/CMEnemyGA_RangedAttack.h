// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility.h"
#include "CMEnemyGA_RangedAttack.generated.h"

class ACMProjectileActor;
/**
 * 적 전용 원거리 공격 어빌리티
 * 타겟을 향해 투사체를 발사합니다.
 */
UCLASS()
class CRIMSONMOON_API UCMEnemyGA_RangedAttack : public UCMEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UCMEnemyGA_RangedAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 발사할 투사체 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<ACMProjectileActor> ProjectileClass;

	/** 투사체에 실어 보낼 데미지 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 발사 애니메이션 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 투사체가 생성될 소켓 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FName MuzzleSocketName;

	/** 기본 데미지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float BaseDamage = 10.0f;

	/** 몽타주 종료 콜백 */
	UFUNCTION()
	void OnMontageEnded();

	/** 몽타주 취소 콜백 */
	UFUNCTION()
	void OnMontageCancelled();

	/** AnimNotify 수신 시 투사체 발사 */
	UFUNCTION()
	void OnFireProjectileEvent(FGameplayEventData Payload);

	/** 타겟을 향한 회전값 계산 */
	FRotator GetRotationToTarget(const FVector& StartLocation) const;
	
};
