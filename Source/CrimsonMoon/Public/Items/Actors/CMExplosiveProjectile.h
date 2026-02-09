// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Actors/CMThrowProjectileActor.h"
#include "CMExplosiveProjectile.generated.h"

class UNiagaraSystem;

/**
 * 시간차 폭발 투척물
 */
UCLASS()
class CRIMSONMOON_API ACMExplosiveProjectile : public ACMThrowProjectileActor
{
	GENERATED_BODY()

public:
	ACMExplosiveProjectile();

protected:
	// 게임 시작 시 타이머를 돌리기 위해 오버라이드
	virtual void BeginPlay() override;

	// 부모 클래스의 즉시 폭발 로직을 무시하기 위해 오버라이드
	virtual void OnProjectileHit(AActor* HitActor) override;

	// 실제 폭발 로직
	void Explode();

protected:
	// 폭발 반경
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionRadius = 500.0f;

	// 폭발 시 보여줄 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;

	// 폭발 효과음
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	TObjectPtr<USoundBase> ExplosionSound;

	// 기폭 시간
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float FuseTime = 3.0f;
    
	// 디버그 드로잉 여부
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowDebugRadius = false;

private:
	// 폭발 타이머 핸들
	FTimerHandle ExplosionTimerHandle;
};
