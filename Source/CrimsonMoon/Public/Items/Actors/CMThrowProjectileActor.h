// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "DataAssets/Consumable/CMDataAsset_ConsumableData.h"
#include "CMThrowProjectileActor.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class CRIMSONMOON_API ACMThrowProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACMThrowProjectileActor();

	// 외부에서 메쉬를 설정할 수 있는 함수
	void SetProjectileMesh(UStaticMesh* NewMesh);

	// 클라이언트 예측용 가짜 발사체로 설정하는 함수
	void InitFakeProjectile();

	// 데이터 에셋의 정보로 발사체 초기화
	void InitializeProjectile(const FCMThrowableData& ThrowableData);

protected:
	virtual void BeginPlay() override;
	
	// Instigator가 복제되었을 때 호출
	virtual void OnRep_Instigator() override;

	// 충돌 감지용 구체
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComponent;

	// 발사체 움직임 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 눈에 보이는 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 캐릭터와 겹쳤을 때 호출될 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 자식에서 오버라이드하여 "부딪히면 할 일"을 정의할 함수
	virtual void OnProjectileHit(AActor* HitActor);
	
public:
	// 어빌리티가 전달해 줄 효과 주문서
	FGameplayEffectSpecHandle ProjectileEffectSpec;

private:
	// 이 발사체가 클라이언트에서 생성한 가짜인지 여부
	bool bIsFake = false;
};
