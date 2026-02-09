// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "CMSTEvaluator_HealthRatio.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct CRIMSONMOON_API FCMSTEvaluator_HealthPercentInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> ContextActor = nullptr;

	// [출력] 계산된 체력 비율
	UPROPERTY(EditAnywhere, Category = "Output")
	float HealthPercent = 1.0f;

	// TreeStart에서 찾은 ASC를 저장해둘 공간
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC = nullptr;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Health Percent Evaluator"))
struct CRIMSONMOON_API FCMSTEvaluator_HealthRatio : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCMSTEvaluator_HealthPercentInstanceData;

protected:
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 초기화 함수 오버라이드 (여기서 캐싱)
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};