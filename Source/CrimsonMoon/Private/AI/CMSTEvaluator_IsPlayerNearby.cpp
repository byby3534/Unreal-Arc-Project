// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CMSTEvaluator_IsPlayerNearby.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "StateTreeExecutionContext.h"

void UCMSTEvaluator_IsPlayerNearby::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	// Context Actor를 적 캐릭터(EnemyBase)로 캐스팅
	ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(ContextActor);
	if (!EnemyCharacter)
	{
		bIsPlayerNearby = false;
		return;
	}

	// AIPerception이 찾아둔 'StateTreeTargetActor'가 있는지 확인
	// (UpdateMainTarget 함수에서 Perception이 감지하면 이 변수에 값을 채워줍니다)
	if (EnemyCharacter->StateTreeTargetActor != nullptr)
	{
		bIsPlayerNearby = true;
	}
	else
	{
		bIsPlayerNearby = false;
	}
}