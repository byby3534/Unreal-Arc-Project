// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "CMSTTask_ActivateAbility.generated.h"

/**
 * State Tree에서 원하는 GAS 어빌리티를 태그로 실행시키는 범용 태스크
 */
UCLASS(DisplayName = "Activate Gameplay Ability", Category = "CrimsonMoon|AI")
class CRIMSONMOON_API UCMSTTask_ActivateAbility : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	// [입력] 어빌리티를 실행할 주체 (AI 캐릭터)
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> ContextActor;

	// [설정] 실행할 어빌리티의 태그 (에디터에서 "Enemy.Ability.Attack" 등을 선택)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag AbilityTagToActivate;

	// [설정] 어빌리티가 끝날 때까지 State Tree를 대기시킬지 여부
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bWaitForAbilityEnd = true;

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};
