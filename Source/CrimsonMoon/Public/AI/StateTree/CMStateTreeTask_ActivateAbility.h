// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "CMStateTreeTask_ActivateAbility.generated.h"

/**
 * State Tree에서 GAS 어빌리티를 태그로 활성화하는 태스크
 */
USTRUCT(meta = (DisplayName = "Activate GAS Ability By Tag", Category = "CM|Ability"))
struct CRIMSONMOON_API FCMStateTreeTask_ActivateAbility : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 입력 데이터 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag AbilityTagToActivate;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};