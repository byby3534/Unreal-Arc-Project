// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CMSTEvaluator_HealthRatio.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySystem/CMAttributeSet.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Controller.h"

void FCMSTEvaluator_HealthRatio::TreeStart(FStateTreeExecutionContext& Context) const
{
	// 데이터 저장소 가져오기
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// 바인딩된 액터 가져오기
	AActor* TargetActor = InstanceData.ContextActor.Get();

	// 유효하지 않으면 캐싱 초기화 후 리턴
	if (!TargetActor)
	{
		InstanceData.CachedASC = nullptr;
		return;
	}

	// 컨트롤러라면 폰으로 변환
	if (AController* Controller = Cast<AController>(TargetActor))
	{
		TargetActor = Controller->GetPawn();
	}

	if (TargetActor)
	{
		// ASC를 찾아 InstanceData에 저장 (캐싱)
		InstanceData.CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	}
	else
	{
		InstanceData.CachedASC = nullptr;
	}
}


void FCMSTEvaluator_HealthRatio::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// 기본값 초기화
	InstanceData.HealthPercent = 1.0f;

	// 캐싱된 ASC 꺼내 쓰기
	UAbilitySystemComponent* ASC = InstanceData.CachedASC.Get();

	// ASC가 없거나 유효하지 않으면 리턴
	if (!IsValid(ASC))
	{
		return;
	}

	bool bFoundCurrent = false;
	bool bFoundMax = false;

	// 값 가져오기
	float CurrentHealth = ASC->GetGameplayAttributeValue(UCMAttributeSet::GetCurrentHealthAttribute(), bFoundCurrent);
	float MaxHealth = ASC->GetGameplayAttributeValue(UCMAttributeSet::GetMaxHealthAttribute(), bFoundMax);

	// 결과 저장
	if (bFoundCurrent && bFoundMax && MaxHealth > 0.f)
	{
		InstanceData.HealthPercent = CurrentHealth / MaxHealth;
	}
}