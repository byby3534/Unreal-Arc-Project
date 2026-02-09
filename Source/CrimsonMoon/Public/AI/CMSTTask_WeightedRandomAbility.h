// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "CMSTTask_WeightedRandomAbility.generated.h"

USTRUCT()
struct CRIMSONMOON_API FCMSTTask_WeightedRandomAbilityInstanceData
{
	GENERATED_BODY()

	// [입력] 어빌리티를 실행할 주체 (Actor 바인딩용)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> ContextActor = nullptr;

	// [임시 저장] 랜덤으로 선택된 어빌리티 태그 (이 변수 덕분에 멀티플레이어/다수 AI에서도 안전함)
	UPROPERTY()
	FGameplayTag SelectedAbilityTag;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Activate Random Ability (Weighted)", Category = "CrimsonMoon|AI"))
struct CRIMSONMOON_API FCMSTTask_WeightedRandomAbility : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCMSTTask_WeightedRandomAbilityInstanceData;

	// 스킬 목록과 가중치 (Key: 태그, Value: 확률 가중치)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TMap<FGameplayTag, float> AbilityWeights;

	// 스킬이 끝날 때까지 기다릴지 여부
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bWaitForAbilityEnd = true;

protected:
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};