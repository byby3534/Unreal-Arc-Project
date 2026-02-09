// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/CMEnemyController.h"
#include "Components/StateTreeAIComponent.h"
#include "Components/StateTreeComponent.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "AIController.h"
#include "Enums/CMEnums.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "AI/CMAITypes.h"

ACMEnemyController::ACMEnemyController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));

	// --- AI 감지 시스템 ---

	// 시각 설정 생성
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.0f;                // 감지 반경
	SightConfig->LoseSightRadius = 2500.0f;            // 감지를 잃는 반경
	SightConfig->PeripheralVisionAngleDegrees = 90.0f; // 시야각
	SightConfig->SetMaxAge(4.0f);                      // 감지 정보 유지 시간

	// 적만 감지하도록 설정
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	// PerceptionComponent에 시각 설정 적용
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	// 주 감각을 시각으로 설정
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	// Team ID 설정
	SetGenericTeamId(FGenericTeamId(static_cast<uint8>(ETeamType::Enemy)));

	UE_LOG(LogTemp, Warning, TEXT("[적 컨트롤러] 생성자 - Team ID 설정: %d (Enemy)"), static_cast<uint8>(ETeamType::Enemy));
}

ETeamAttitude::Type ACMEnemyController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* PawnToCheck = Cast<const APawn>(&Other);
	if (!PawnToCheck)
	{
		return ETeamAttitude::Neutral;
	}

	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(PawnToCheck->GetController());
	if (!OtherTeamAgent)
	{
		return ETeamAttitude::Neutral;
	}

	const FGenericTeamId MyTeamId = GetGenericTeamId();
	const FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();

	ETeamAttitude::Type Attitude;
	if (OtherTeamId < MyTeamId)
	{
		Attitude = ETeamAttitude::Hostile;
	}
	else
	{
		Attitude = ETeamAttitude::Friendly;
	}

	return Attitude;
}

void ACMEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[적 컨트롤러] Pawn 빙의 완료 - Pawn: %s, 내 Team ID: %d"),
			*InPawn->GetName(), GetGenericTeamId().GetId());
	}

	// AI Perception 콜백 등록
	if (AIPerceptionComponent && HasAuthority())
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetPerceptionUpdated);
		UE_LOG(LogTemp, Warning, TEXT("[적 컨트롤러] AI Perception 콜백 등록 완료"));
	}
}

void ACMEnemyController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!HasAuthority())
	{
		return;
	}

	// 대상의 Team ID 확인
	FString TargetTeamInfo = TEXT("알 수 없음");
	if (APawn* TargetPawn = Cast<APawn>(Actor))
	{
		if (IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController()))
		{
			TargetTeamInfo = FString::Printf(TEXT("Team %d"), TargetTeamAgent->GetGenericTeamId().GetId());
		}
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		if (SensedEnemySet.Contains(Actor))
		{
			return;
		}

		SensedEnemySet.Add(Actor);

		UE_LOG(LogTemp, Log, TEXT("AI 감지 [%s]: 타겟 감지 -> %s (목록 %d개). 베스트 타겟 갱신."), *GetName(), *Actor->GetName(), SensedEnemySet.Num());
		UpdateMainTarget();
	}
	else
	{
		float LoseRadius = (SightConfig) ? SightConfig->LoseSightRadius : 2500.0f;

		APawn* ControlledPawn = GetPawn();
		if (!ControlledPawn)
		{
			return;
		}

		float CurrentDistSq = FVector::DistSquared(ControlledPawn->GetActorLocation(), Actor->GetActorLocation());

		// 플레이어가 시야각 밖으로 나가더라도 몬스터가 감지를 잃는 반경 밖으로 나가지 않는다면 추적 상태를 유지함.
		if (CurrentDistSq <= (LoseRadius * LoseRadius))
		{
			return;
		}

		// 거리가 정말로 멀어졌을 때만 삭제 진행
		int32 RemovedCount = SensedEnemySet.Remove(Actor);

		if (RemovedCount == 0)
		{
			RemovedCount = SensedEnemySet.Remove(nullptr);
		}

		if (RemovedCount == 0)
		{
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("AI 감지 [%s]: 타겟 소실 -> %s (남은 목록 %d개). 베스트 타겟 갱신."), *GetName(), *Actor->GetName(), SensedEnemySet.Num());
		UpdateMainTarget();
	}
}

void ACMEnemyController::UpdateMainTarget()
{
	ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(GetPawn());
	if (!EnemyCharacter)
	{
		return;
	}

	AActor* BestTarget = nullptr;
	float MinDistanceSq = FLT_MAX;
	const FVector MyLocation = EnemyCharacter->GetActorLocation();

	// 목록에서 가장 가까운 유효 액터를 찾기
	for (const TObjectPtr<AActor>& TempActor : SensedEnemySet)
	{
		if (TempActor)
		{
			const float DistanceSq = FVector::DistSquared(MyLocation, TempActor->GetActorLocation());
			if (DistanceSq < MinDistanceSq)
			{
				MinDistanceSq = DistanceSq;
				BestTarget = TempActor.Get();
			}
		}
	}

	if (BestTarget)
	{
		if (EnemyCharacter->StateTreeTargetActor != BestTarget)
		{
			// Character의 StateTreeTargetActor 설정
			EnemyCharacter->StateTreeTargetActor = BestTarget;
			UE_LOG(LogTemp, Log, TEXT("AI 감지 [%s]: 메인 타겟 설정/교체 -> %s (가장 가까운 적)"),
				*GetName(), *BestTarget->GetName());
			EnemyCharacter->SetAIState(ECMEnemyState::Combat);
		}
	}
	else
	{
		if (EnemyCharacter->StateTreeTargetActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("AI 감지 [%s]: 모든 타겟 소실. Idle 상태로 변경."), *GetName());
			EnemyCharacter->StateTreeTargetActor = nullptr;
			EnemyCharacter->SetAIState(ECMEnemyState::Idle);
		}
	}
}