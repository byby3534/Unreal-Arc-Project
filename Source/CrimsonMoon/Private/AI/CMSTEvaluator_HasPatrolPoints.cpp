// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CMSTEvaluator_HasPatrolPoints.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "StateTreeExecutionContext.h"

void UCMSTEvaluator_HasPatrolPoints::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	const ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(ContextActor);
	if (EnemyCharacter)
	{
		bHasPatrolPoints = !EnemyCharacter->GetPatrolPoints().IsEmpty();
	}
	else
	{
		bHasPatrolPoints = false;
	}
}