// Copyright CrimsonMoon Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Skill/Projectile/CMProjectileActor.h"
#include "CMAdventActor.generated.h"

/**
 * Advent 스킬의 장판 액터
 * - 지정된 위치에 고정되어 도트 데미지 적용
 * - ProjectileMovement 사용하지 않음
 */
UCLASS()
class CRIMSONMOON_API ACMAdventActor : public ACMProjectileActor
{
	GENERATED_BODY()

public:
	ACMAdventActor();

	// 장판 초기화 (ProjectileMovement 대신 사용)
	virtual void InitProjectileTransform(const FTransform& SpawnTransform) override;

	// 도트 데미지 정보 설정
	void SetDotDamageInfo(const TSubclassOf<UGameplayEffect>& InDotDamageEffect,
		float InDotDamageMultiplier,
		float InDotInterval,
		float InDotDuration,
		float InExplosionRadius);

	// 도트 타이머 시작
	void StartDotTimer();

	// 디버그 드로우 설정
	void SetDebugDrawSettings(bool bEnable, float Duration);

protected:
	virtual void PoolActivate_Implementation() override;
	virtual void PoolReturn_Implementation() override;

private:
	/** 도트 데미지 GameplayEffect */
	UPROPERTY()
	TSubclassOf<UGameplayEffect> DotDamageEffect;

	/** 도트 데미지 배율 */
	float DotDamageMultiplier = 0.5f;

	/** 도트 데미지 틱 간격 (초) */
	float DotInterval = 0.5f;

	/** 장판 지속 시간 (초) */
	float DotDuration = 5.0f;

	/** 폭발 범위 (반경) */
	float ExplosionRadius = 500.0f;

	/** 디버그 드로우 활성화 여부 */
	bool bEnableDebugDraw = false;

	/** 디버그 드로우 지속 시간 (초) */
	float DebugDrawDuration = 3.0f;

	/** 도트 타이머 */
	FTimerHandle DotTimerHandle;

	/** 도트 데미지 틱 콜백 */
	UFUNCTION()
	void OnDotTick();

	/** 도트 데미지 적용 */
	void ApplyDotDamage();
};