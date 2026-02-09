// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "CMSTTask_FindPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMSTTask_FindPatrolPoint : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	// [입력] 'Context.Actor'에 바인딩
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> ContextActor;

	// [입력/출력] 'PatrolPointIndex' 파라미터 변수에 바인딩 (값을 읽고 수정함)
	UPROPERTY(EditAnywhere, Category = "Input|Output")
	int32 PatrolPointIndex = 0;

	// [출력] 'NextPatrolLocation' 파라미터 변수에 바인딩
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector NextPatrolLocation = FVector::ZeroVector;

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
};