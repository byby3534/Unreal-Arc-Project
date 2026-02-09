// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPGAbility_BaseAttack.generated.h"

class ACMProjectileActor;
/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMPGAbility_BaseAttack : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPGAbility_BaseAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/**
	 * @brief 조준점을 기반으로 투사체의 최종 스폰 트랜스폼을 계산하여 반환합니다.
	 * 블루프린트에서 순수 함수(Pure)로 호출할 수 있습니다.
	 * @return 계산된 최종 스폰 트랜스폼
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CM|Projectile")
	FTransform CalculateProjectileSpawnTransform();

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnFireProjectileEvent(FGameplayEventData Payload);

	void FireProjectile();

	UPROPERTY(EditDefaultsOnly, Category = "CM|Projectile")
	TSubclassOf<ACMProjectileActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess=true))
	TObjectPtr<UAnimMontage> FireProjectileMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Projectile")
	FName SpawnSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Projectile")
	FName ComponentTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Projectile")
	FTransform SpawnTransformOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Projectile")
	float LineTraceRange = 10000.0f;

	// 데미지 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|DamageInfo", meta = (AllowPrivateAccess=true))
	TSubclassOf<UGameplayEffect> DamageEffect;

	// SetByCaller로 전달할 BaseDamage = WeaponDamage * SkillDamageMultiplier
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|DamageInfo", meta = (ClampMin = "0.0"))
	float SkillDamageMultiplier = 1.0f;

	// 직접 태그 쓰는 대신 GameplayCue 태그가 포함된 지속형 Effect 사용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Effects")
	TSubclassOf<UGameplayEffect> SoundEffectClass;

	// 적용된 루프 사운드 GE를 추적하기 위한 핸들
	FActiveGameplayEffectHandle SoundEffectHandle;
};