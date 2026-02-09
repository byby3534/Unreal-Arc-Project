// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "CMSTEvaluator_IsPlayerNearby.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMSTEvaluator_IsPlayerNearby : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

public:
	// [입력] AI 캐릭터 (Context Actor)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> ContextActor;

	// [설정] 감지 거리
	UPROPERTY(EditAnywhere, Category = "Settings")
	float DetectRange = 1000.0f;

	// [출력] 플레이어가 범위 내에 있으면 True
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsPlayerNearby = false;

protected:
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};