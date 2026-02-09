// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/CMSTEvaluator_DistanceToTarget.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "StateTreeExecutionContext.h"

void FCMDistanceToTargetEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// 에디터에서 바인딩된 ContextActor를 가져옴
	AActor* SourceActor = InstanceData.ContextActor;

	// SourceActor가 내 적 캐릭터인지 확인 (Cast)
	if (ACMEnemyCharacterBase* Enemy = Cast<ACMEnemyCharacterBase>(SourceActor))
	{
		// 적 캐릭터 안에 있는 TargetActor 가져오기
		if (AActor* Target = Enemy->StateTreeTargetActor)
		{
			// 거리 계산
			InstanceData.DistanceToTarget = Enemy->GetDistanceTo(Target);
			return;
		}
	}
	else
	{
		// ContextActor가 연결 안 됐거나, 엉뚱한 액터가 들어온 경우
		static bool bLogWarningOnce = false;
		if (!bLogWarningOnce)
		{
			UE_LOG(LogTemp, Warning, TEXT("ContextActor Input is Invalid or NULL"));
			bLogWarningOnce = true;
		}
	}

	// [실패 시 기본값]
	InstanceData.DistanceToTarget = 99999.0f;
}