// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "CMSTEvaluator_HasPatrolPoints.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMSTEvaluator_HasPatrolPoints : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

public:
	// [입력] Context Actor로 바인딩
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> ContextActor;

	// [출력] 'bHasPatrolPoints' 변수로 바인딩
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasPatrolPoints = false;

protected:
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};