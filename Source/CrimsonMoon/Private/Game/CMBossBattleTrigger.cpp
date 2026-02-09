#include "Game/CMBossBattleTrigger.h"
#include "Components/BoxComponent.h"
#include "Game/CMGameStateMainStageV2.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"

ACMBossBattleTrigger::ACMBossBattleTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

	bHasTriggered = false;
}

void ACMBossBattleTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACMBossBattleTrigger::OnOverlapBegin);
	}
}

void ACMBossBattleTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasTriggered || !BossReference)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	if (OtherActor && OtherActor->IsA(ACMPlayerCharacterBase::StaticClass()))
	{
		if (UWorld* World = GetWorld())
		{
			ACMGameStateMainStageV2* CMGameState = World->GetGameState<ACMGameStateMainStageV2>();
			if (CMGameState)
			{
				CMGameState->RegisterBoss(BossReference);

				bHasTriggered = true;

				// 필요하다면 트리거 비활성화 또는 파괴
				// TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				UE_LOG(LogTemp, Log, TEXT("Boss Battle Triggered: Boss Registered to GameState"));
			}
		}
	}
}