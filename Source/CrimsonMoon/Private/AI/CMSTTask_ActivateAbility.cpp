// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CMSTTask_ActivateAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "StateTreeExecutionContext.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"

EStateTreeRunStatus UCMSTTask_ActivateAbility::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	if (!ContextActor || !AbilityTagToActivate.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	// ASC 가져오기
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Payload 만들기
	FGameplayEventData Payload;
	Payload.Instigator = ContextActor;
	Payload.EventTag = AbilityTagToActivate;

	if (ACMEnemyCharacterBase* Enemy = Cast<ACMEnemyCharacterBase>(ContextActor))
	{
		if (IsValid(Enemy->StateTreeTargetActor))
		{
			Payload.Target = Enemy->StateTreeTargetActor;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AI Task] [%s] 타겟이 유효하지 않습니다. (Target is Invalid)"), *GetNameSafe(Enemy));
			Payload.Target = nullptr;
		}
	}

	// 어빌리티 발동 (SendGameplayEventToActor)
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(ContextActor, AbilityTagToActivate, Payload);

	// 실행 여부 체크 (이벤트 방식은 리턴값이 없어서, 태그로 활성 여부 확인)
	// (보통 이벤트 쏘면 바로 실행되니 Running 반환)

	if (!bWaitForAbilityEnd)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UCMSTTask_ActivateAbility::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	// 어빌리티가 종료되었는지 확인하는 로직
	// (가장 간단한 방법: 해당 태그를 가진 어빌리티가 활성화 상태인지 체크)
	// 주의: 쿨타임 등으로 인해 발동 실패한 경우 등 복잡한 케이스는 생략하고 '작동 중인가'만 확인합니다.

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor);
	if (ASC)
	{
		// 해당 태그를 가진 어빌리티 인스턴스들을 찾음
		TArray<FGameplayAbilitySpec*> Specs;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(AbilityTagToActivate), Specs);

		bool bIsActive = false;
		for (const FGameplayAbilitySpec* Spec : Specs)
		{
			if (Spec->IsActive())
			{
				bIsActive = true;
				break;
			}
		}

		// 아직 활성화 상태면 계속 Running
		if (bIsActive)
		{
			return EStateTreeRunStatus::Running;
		}
	}

	// 어빌리티가 끝났으면 성공 처리 -> 다음 State로 넘어감
	return EStateTreeRunStatus::Succeeded;
}