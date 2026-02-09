// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Task/AbilityTask_WaitRepeat.h"
#include "TimerManager.h"

UAbilityTask_WaitRepeat* UAbilityTask_WaitRepeat::WaitRepeat(
	UGameplayAbility* OwningAbility,
	float Interval,
	float TotalDuration,
	bool bStartImmediately)
{
	UAbilityTask_WaitRepeat* MyObj = NewAbilityTask<UAbilityTask_WaitRepeat>(OwningAbility);
	MyObj->TickInterval = Interval;
	MyObj->Duration = TotalDuration;
	MyObj->bExecuteImmediately = bStartImmediately;
	return MyObj;
}

void UAbilityTask_WaitRepeat::Activate()
{
	Super::Activate();

	if (!IsValid(Ability))
	{
		EndTask();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		EndTask();
		return;
	}

	// 반복 타이머 설정
	World->GetTimerManager().SetTimer(
		RepeatTimerHandle,
		this,
		&ThisClass::ExecuteTick,
		TickInterval,
		true,                                     // Loop
		bExecuteImmediately ? 0.0f : TickInterval // 즉시 실행 여부
		);

	// 총 지속시간이 설정되어 있으면 종료 타이머 설정
	if (Duration > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			FinishTimerHandle,
			this,
			&ThisClass::FinishTask,
			Duration,
			false // 한 번만
			);
	}
}

void UAbilityTask_WaitRepeat::OnDestroy(bool bInOwnerFinished)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RepeatTimerHandle);
		World->GetTimerManager().ClearTimer(FinishTimerHandle);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitRepeat::ExecuteTick()
{
	// 어빌리티가 여전히 유효한지 확인
	if (!IsValid(Ability) || !Ability->IsActive())
	{
		EndTask();
		return;
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast();
	}
}

void UAbilityTask_WaitRepeat::FinishTask()
{
	// 어빌리티가 여전히 유효하면 브로드캐스트
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinished.Broadcast();
	}

	EndTask();
}