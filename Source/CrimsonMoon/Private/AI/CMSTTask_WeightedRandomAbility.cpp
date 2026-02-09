// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CMSTTask_WeightedRandomAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Controller.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"

EStateTreeRunStatus FCMSTTask_WeightedRandomAbility::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// ContextActor 확인
	AActor* AvatarActor = InstanceData.ContextActor.Get();
	if (!AvatarActor)
	{
		// 바인딩이 안 되어 있으면 Owner라도 가져와봄
		AvatarActor = Cast<AActor>(Context.GetOwner());
	}

	// 컨트롤러라면 폰(캐릭터)을 찾아옴
	if (AController* Controller = Cast<AController>(AvatarActor))
	{
		AvatarActor = Controller->GetPawn();
	}

	if (!AvatarActor)
	{
		UE_LOG(LogTemp, Error, TEXT("[WeightedRandomTask] AvatarActor를 찾을 수 없습니다."));
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AvatarActor);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 가중치 맵이 비어있는지 확인
	if (!ensureMsgf(!AbilityWeights.IsEmpty(), TEXT("[WeightedRandomTask] AbilityWeights가 비어있습니다! StateTree 에셋에서 어빌리티와 가중치를 설정해주세요.")))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 가중치 랜덤 뽑기 알고리즘
	float TotalWeight = 0.f;
	for (const auto& Pair : AbilityWeights)
	{
		if (Pair.Value > 0.f)
		{
			TotalWeight += Pair.Value;
		}
	}

	if (TotalWeight <= 0.f)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 0 ~ 총합 사이의 랜덤 실수 뽑기
	const float RandomValue = FMath::FRandRange(0.f, TotalWeight);
	float CurrentSum = 0.f;

	// 초기화
	InstanceData.SelectedAbilityTag = FGameplayTag::EmptyTag;

	for (const auto& Pair : AbilityWeights)
	{
		CurrentSum += Pair.Value;

		// 랜덤 값이 누적 합보다 작거나 같으면 당첨!
		if (RandomValue <= CurrentSum)
		{
			InstanceData.SelectedAbilityTag = Pair.Key;
			break;
		}
	}

	if (!InstanceData.SelectedAbilityTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 타겟 정보 담기 (Payload)
	FGameplayEventData Payload;
	Payload.Instigator = AvatarActor;
	Payload.EventTag = InstanceData.SelectedAbilityTag;

	if (ACMEnemyCharacterBase* Enemy = Cast<ACMEnemyCharacterBase>(AvatarActor))
	{
		if (IsValid(Enemy->StateTreeTargetActor))
		{
			Payload.Target = Enemy->StateTreeTargetActor;
		}
	}

	// 선택된 어빌리티 실행
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarActor, InstanceData.SelectedAbilityTag, Payload);

	// 디버깅용 로그 (확인 후 주석 처리)
	// UE_LOG(LogTemp, Log, TEXT("[WeightedRandom] %s 발동! (Random: %.1f/%.1f)"), *InstanceData.SelectedAbilityTag.ToString(), RandomValue, TotalWeight);

	// 대기 옵션이 꺼져있으면 바로 성공 처리
	if (!bWaitForAbilityEnd)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FCMSTTask_WeightedRandomAbility::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// EnterState에서 아무것도 못 뽑았으면 실패
	if (!InstanceData.SelectedAbilityTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	// 액터 다시 찾기 (매 틱 검사)
	AActor* AvatarActor = InstanceData.ContextActor.Get();
	if (AController* Controller = Cast<AController>(AvatarActor))
	{
		AvatarActor = Controller->GetPawn();
	}

	if (!AvatarActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AvatarActor);

	// 아까 뽑은 그 스킬(SelectedAbilityTag)이 아직도 돌고 있는지 확인
	if (ASC)
	{
		TArray<FGameplayAbilitySpec*> Specs;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(InstanceData.SelectedAbilityTag), Specs);

		bool bIsActive = false;
		for (const FGameplayAbilitySpec* Spec : Specs)
		{
			if (Spec->IsActive())
			{
				bIsActive = true;
				break;
			}
		}

		// 아직 실행 중이면 계속 Running
		if (bIsActive)
		{
			return EStateTreeRunStatus::Running;
		}
	}

	// 실행 끝났으면 성공
	return EStateTreeRunStatus::Succeeded;
}