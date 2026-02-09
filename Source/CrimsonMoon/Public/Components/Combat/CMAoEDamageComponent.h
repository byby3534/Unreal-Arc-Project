// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "CMAoEDamageComponent.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

/**
 * AoE (Area of Effect) 데미지를 처리하는 컴포넌트
 * 투사체 액터에 붙여서 광역 피해 입힘
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMSONMOON_API UCMAoEDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMAoEDamageComponent();

	/**
	 * @brief 지정된 위치에서 반경 내 모든 액터에게 광역 피해를 적용하는 함수
	 * @param HitResult 히트에 대한 정보 (폭발 중심 위치 - ImpactPoint)
	 * @param InstigatorActor 피해를 발생시킨 액터
	 * @param SourceASC GE를 적용하는 주체 AbilitySystemComponent
	 * @param InDamageEffect 적용할 데미지 이펙트
	 * @param InBaseDamage SetByCaller로 전달할 기본 데미지 값
	 * @param AOERadius 탐색 범위 강제 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "CM|AoE Damage")
	void ApplyAoEDamage(const FHitResult& HitResult, AActor* InstigatorActor, UAbilitySystemComponent* SourceASC, TSubclassOf<UGameplayEffect> InDamageEffect, float InBaseDamage, float AOERadius = 0);

	/**
	 * @brief 반경 내의 모든 Pawn 액터를 가져오는 함수
	 * @param Location 중심 위치
	 * @param OutActors 발견된 액터들이 저장될 배열
	 * @param AOERadius 탐색 범위 강제 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "CM|AoE Damage")
	void GetActorsInRadius(FVector Location, TArray<AActor*>& OutActors, float AOERadius = 0);

protected:
	virtual void BeginPlay() override;

	/**
	 * @brief 액터가 유효한 타겟인지 검사하는 함수
	 * @param Actor 검사할 액터
	 * @param InstigatorActor 데미지를 발생시킨 액터(가해 액터)
	 * @return 유효한 타겟이면 true
	 */
	bool IsValidTarget(AActor* Actor, AActor* InstigatorActor) const;

	// 폭발 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|AoE Damage", meta = (ClampMin = "0.0"))
	float ExplosionRadius = 300.0f;

	// 발사자(Instigator)를 데미지에서 제외할지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|AoE Damage")
	bool bIgnoreInstigator = true;

	// 디버그 드로잉 활성화
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|AoE Damage|Debug")
	bool bDebugDraw = false;

	// 디버그 드로잉 지속 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|AoE Damage|Debug", meta = (EditCondition = "bDebugDraw"))
	float DebugDrawDuration = 2.0f;
};