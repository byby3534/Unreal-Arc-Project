// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/CMSTTask_SetMovementSpeed.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FCMSTTask_SetMovementSpeed::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// 1. 컴포넌트 캐싱 (최초 1회 또는 유효하지 않을 때)
	if (InstanceData.CachedMovementComp == nullptr)
	{
		// 바인딩된 ContextActor 가져오기
		AActor* TargetActor = InstanceData.ContextActor.Get();

		if (TargetActor)
		{
			// 만약 입력된 액터가 컨트롤러라면, 폰(Pawn)을 가져오도록 처리 (안전장치)
			if (AController* Controller = Cast<AController>(TargetActor))
			{
				TargetActor = Controller->GetPawn();
			}

			// 캐릭터로 캐스팅해서 무브먼트 컴포넌트 찾기
			if (ACharacter* Character = Cast<ACharacter>(TargetActor))
			{
				InstanceData.CachedMovementComp = Character->GetCharacterMovement();
			}
		}
	}

	// 2. 이동 속도 변경
	if (InstanceData.CachedMovementComp)
	{
		InstanceData.CachedMovementComp->MaxWalkSpeed = InstanceData.TargetSpeed;
	}

	return EStateTreeRunStatus::Running;
}