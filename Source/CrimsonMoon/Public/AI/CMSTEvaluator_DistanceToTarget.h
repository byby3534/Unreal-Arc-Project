// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "CMSTEvaluator_DistanceToTarget.generated.h"

// 인스턴스 데이터: 각 몬스터별로 따로 저장될 변수들
USTRUCT()
struct CRIMSONMOON_API FCMDistanceToTargetEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> ContextActor = nullptr;

	// 출력(Output)으로 사용할 거리 변수
	UPROPERTY(EditAnywhere, Category = "Output", meta = (Output))
	float DistanceToTarget = 0.0f;
};

USTRUCT(meta = (DisplayName = "Distance To Target"))
struct CRIMSONMOON_API FCMDistanceToTargetEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCMDistanceToTargetEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};