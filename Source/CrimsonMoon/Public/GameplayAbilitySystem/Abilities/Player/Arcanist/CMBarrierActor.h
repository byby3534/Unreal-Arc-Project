// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CMBarrierActor.generated.h"

class USphereComponent;
class UNiagaraComponent;
/**
 * 아케이니스트 막기(Block) 어빌리티 사용 시 소환되는 배리어 액터입니다.
 * - 적의 투사체나 공격을 막는 충돌체(SphereCollision)를 가집니다.
 * - 배리어의 외형을 담당하는 나이아가라 이펙트를 재생합니다.
 */
UCLASS()
class CRIMSONMOON_API ACMBarrierActor : public AActor
{
	GENERATED_BODY()
	
public:
	ACMBarrierActor();

protected:
	virtual void BeginPlay() override;

	// 충돌 처리를 위한 구체 컴포넌트 (루트 컴포넌트)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereCollision;

	// 배리어의 시각 효과를 담당할 나이아가라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> BarrierEffectComponent;

	// 배리어 충돌 영역 시각화 (디버그/테스트용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bVisualizeBarrier = false;

private:
	// 배리어가 공격을 받았을 때 호출되는 함수
	UFUNCTION()
	void OnBarrierHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Owner의 ASC에 블록 이벤트 발송
	void SendBlockEventToOwner(AActor* Attacker) const;
};