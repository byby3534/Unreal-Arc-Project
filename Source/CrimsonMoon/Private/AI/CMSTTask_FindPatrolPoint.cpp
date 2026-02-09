// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CMSTTask_FindPatrolPoint.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus UCMSTTask_FindPatrolPoint::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	const ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(ContextActor);
	if (!EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	const TArray<AActor*> PatrolPoints = EnemyCharacter->GetPatrolPoints();
	if (PatrolPoints.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 현재 인덱스로 위치 가져오기 (입력값 PatrolPointIndex 사용)
	AActor* TargetPatrolPoint = PatrolPoints[PatrolPointIndex];
	NextPatrolLocation = TargetPatrolPoint->GetActorLocation(); // [출력] NextPatrolLocation 설정

	// 다음 인덱스 계산
	PatrolPointIndex = (PatrolPointIndex + 1) % PatrolPoints.Num(); // [입/출력] PatrolPointIndex 수정

	return EStateTreeRunStatus::Succeeded;
}