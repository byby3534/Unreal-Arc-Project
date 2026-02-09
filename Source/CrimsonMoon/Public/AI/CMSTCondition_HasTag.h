// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeConditionBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "CMSTCondition_HasTag.generated.h"

/**
 * Actor가 특정 Gameplay Tag를 가지고 있는지 검사합니다.
 */
UCLASS()
class CRIMSONMOON_API UCMSTCondition_HasTag : public UStateTreeConditionBlueprintBase
{
	GENERATED_BODY()

public:
	// [입력] 검사할 대상 (AI 자신)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> ContextActor;

	// [설정] 찾을 태그
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag TagToCheck;

	// [설정] 반대로 검사할지 (태그가 '없으면' True)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;

protected:
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};