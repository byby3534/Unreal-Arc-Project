// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Interfaces/CMPoolableObjectInterface.h"
#include "CMProjectileActor.generated.h"

class UCMAoEDamageComponent;
class UGameplayEffect;
struct FGameplayEffectSpecHandle;
class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class CRIMSONMOON_API ACMProjectileActor : public AActor, public ICMPoolableObjectInterface
{
	GENERATED_BODY()

public:
	ACMProjectileActor();

	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PoolActivate_Implementation() override;
	virtual void PoolReturn_Implementation() override;
	virtual bool IsAvailable_Implementation() const override;
	virtual void ReleaseToPool_Implementation() override;

	// 서버에서 호출할 초기화 함수
	virtual void InitProjectileTransform(const FTransform& SpawnTransform);

	// 투사체에 데미지 정보 설정
	void SetDamageInfo(const TSubclassOf<UGameplayEffect>& InDamageEffect, const float InBaseDamage);

	// 투사체 Hit 시 재생할 GameplayCue 태그 설정
	void SetHitGameplayCueTag(const FGameplayTag& InHitCueTag);

	// 단일 타겟 Hit 처리
	void ProcessSingleTargetHit(AActor* HitActor, const FHitResult& HitResult, APawn* SourcePawn, APawn* TargetPawn);

protected:
	virtual void BeginPlay() override;
	virtual void LifeSpanExpired() override;

	// 모든 클라이언트에서 나이아가라 컴포넌트 활성화
	UFUNCTION(NetMulticast, Reliable)
	void MulticastActivateNiagara();

	// 모든 클라이언트에서 나이아가라 컴포넌트 비활성화
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeactivateNiagara();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollisionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCMAoEDamageComponent> AoEComponent;

	// 오브젝트 풀에서 스폰된 액터의 활성화 상태 시간
	float LifeSpan = 5.f;

	// 데미지 이펙트, FireProjectile 태스크에서 받아옴
	TSubclassOf<UGameplayEffect> DamageEffect;

	// 기본 데미지, FireProjectile 태스크에서 받아옴
	float BaseDamage = 0.0f;

private:
	// 풀에서 사용 가능한 상태인지.
	bool bIsAvailablePool = true;

	// Hit 시 실행할 GameplayCue 태그
	FGameplayTag HitGameplayCueTag;
};