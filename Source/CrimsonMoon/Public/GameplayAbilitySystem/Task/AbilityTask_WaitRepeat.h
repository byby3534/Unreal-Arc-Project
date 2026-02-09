// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitRepeat.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitRepeatDelegate);

/**
 * 일정 시간 간격으로 델리게이트를 실행하는 태스크
 * 어빌리티가 끝나면 자동으로 멈춤
 */
UCLASS()
class CRIMSONMOON_API UAbilityTask_WaitRepeat : public UAbilityTask
{
	GENERATED_BODY()

public:
	/**
	 * 일정 간격으로 반복 실행하는 태스크 생성
	 * @param OwningAbility 이 태스크를 실행하는 어빌리티
	 * @param Interval 반복 간격 (초)
	 * @param TotalDuration 총 지속시간 (초), 0이면 무한 반복
	 * @param bStartImmediately true면 즉시 첫 실행, false면 Interval 후 첫 실행
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitRepeat* WaitRepeat(
		UGameplayAbility* OwningAbility,
		float Interval = 0.1f,
		float TotalDuration = 0.0f,
		bool bStartImmediately = true
		);

	// 매 간격마다 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FWaitRepeatDelegate OnTick;

	// 총 지속시간이 끝났을 때 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FWaitRepeatDelegate OnFinished;

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	FTimerHandle RepeatTimerHandle;
	FTimerHandle FinishTimerHandle;

	float TickInterval;
	float Duration;
	bool bExecuteImmediately;

	// 반복 실행 함수
	UFUNCTION()
	void ExecuteTick();

	// 종료 함수
	UFUNCTION()
	void FinishTask();
};