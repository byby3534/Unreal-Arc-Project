// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Actors/CMThrowProjectileActor.h"
#include "CMThrowActor_MolotovProjectile.generated.h"

class ACMZoneEffectActor;
class UGameplayEffect;

/**
 * 닿으면 터지면서 장판(Zone)을 생성하는 투척체
 */
UCLASS()
class CRIMSONMOON_API ACMThrowActor_MolotovProjectile : public ACMThrowProjectileActor
{
	GENERATED_BODY()

public:
	ACMThrowActor_MolotovProjectile();

protected:
		
	virtual void BeginPlay() override;
	
	// 부모의 함수 오버라이드
	virtual void OnProjectileHit(AActor* HitActor) override;

	// 바닥에 부딪혔을 때 호출될 함수
	UFUNCTION()
	void OnGroundHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	// 생성할 화염 장판 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Molotov")
	TSubclassOf<ACMZoneEffectActor> FireZoneClass;

	// 장판에 적용할 게임플레이 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Molotov")
	TSubclassOf<UGameplayEffect> ZoneEffectClass;

	// 장판 지속 시간
	UPROPERTY(EditDefaultsOnly, Category = "Molotov")
	float FireDuration = 10.0f;
};
