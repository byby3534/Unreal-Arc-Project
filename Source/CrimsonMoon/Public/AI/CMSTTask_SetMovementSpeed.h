// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "CMSTTask_SetMovementSpeed.generated.h"


class UCharacterMovementComponent;

USTRUCT()
struct CRIMSONMOON_API FCMSTTask_SetMovementSpeedInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> ContextActor = nullptr;

	// 에디터에서 설정할 이동 속도 (기본값: 600)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float TargetSpeed = 600.0f;

	// 성능 최적화를 위해 무브먼트 컴포넌트를 캐싱해둘 공간
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CachedMovementComp = nullptr;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Set Movement Speed"))
struct CRIMSONMOON_API FCMSTTask_SetMovementSpeed : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCMSTTask_SetMovementSpeedInstanceData;
	
protected:
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	// 상태에 진입할 때(Enter) 속도를 딱 한 번 바꿉니다.
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};