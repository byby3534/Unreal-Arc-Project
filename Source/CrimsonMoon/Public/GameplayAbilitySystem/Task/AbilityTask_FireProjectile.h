// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_FireProjectile.generated.h"

class CMProjectileActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnedProjectileActorDelegate, ACMProjectileActor*, SpawnedActor);

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UAbilityTask_FireProjectile : public UAbilityTask
{
	GENERATED_BODY()

public:
	// 태스크가 성공적으로 액터를 스폰하고 활성화했을 때 호출될 델리게이트
	UPROPERTY(BlueprintAssignable)
	FSpawnedProjectileActorDelegate OnSuccess;

	// 태스크가 어떤 이유로든 실패했을 때 호출될 델리게이트
	UPROPERTY(BlueprintAssignable)
	FSpawnedProjectileActorDelegate OnFailed;

	/**
	 * 스킬 발사체를 스폰하고 초기화 후 활성화하는 어빌리티 태스크
	 * @param OwningAbility 이 태스크를 실행하는 어빌리티
	 * @param ProjectileClass 스폰할 발사체의 클래스
	 * @param SpawnTransform 발사체가 스폰될 위치와 방향
	 * @param DamageEffect 투사체가 적용할 데미지 이펙트
	 * @param BaseDamage 기본 데미지 값
	 */
	UFUNCTION(BlueprintCallable, Category = "GG|Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_FireProjectile* FireProjectile(
		UGameplayAbility* OwningAbility,
		TSubclassOf<ACMProjectileActor> ProjectileClass,
		FTransform SpawnTransform,
		TSubclassOf<UGameplayEffect> DamageEffect,
		float BaseDamage = 10.0f
		);

private:
	virtual void Activate() override;

	UPROPERTY()
	TSubclassOf<ACMProjectileActor> SkillActorClass;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffect;

	FTransform SpawnTransform;
	float BaseDamage;
};