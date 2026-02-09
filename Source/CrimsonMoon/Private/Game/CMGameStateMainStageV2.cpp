#include "Game/CMGameStateMainStageV2.h"
#include "Game/CMGameModeMainStage.h"
#include "Engine/World.h"
#include "Controllers/CMPlayerController.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Game/GameInitialization/CMGameInstance.h"

ACMGameStateMainStageV2::ACMGameStateMainStageV2() 
{
	bReplicates = true;
}

void ACMGameStateMainStageV2::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACMGameStateMainStageV2, CurrentBoss);
}

void ACMGameStateMainStageV2::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 GI에 저장된 카운트를 복원
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (UCMGameInstance* CMGI = Cast<UCMGameInstance>(World->GetGameInstance()))
			{
				int32 LoadedCurrent = 0;
				int32 LoadedDeath = 0;
				CMGI->LoadMainStageCounts(LoadedCurrent, LoadedDeath);

				// 유효한 값이면 복원
				if (LoadedCurrent >= 0 && LoadedDeath >= 0)
				{
					CurrentPlayerCount = LoadedCurrent;
					DeathCount = LoadedDeath;
					UE_LOG(LogTemp, Log, TEXT("GameStateMainStageV2::BeginPlay - Loaded counts from GI: Current=%d, Death=%d"), CurrentPlayerCount, DeathCount);
				}
			}
		}
	}
}

void ACMGameStateMainStageV2::IncrementConnectedPlayerCount()
{
	++CurrentPlayerCount;
	UE_LOG(LogTemp, Log, TEXT("ConnectedPlayerCount Incremented: %d"), CurrentPlayerCount);

	// 서버에서만 GI에 저장
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (UCMGameInstance* CMGI = Cast<UCMGameInstance>(World->GetGameInstance()))
			{
				CMGI->SaveMainStageCounts(CurrentPlayerCount, DeathCount);
			}
		}
	}
}

void ACMGameStateMainStageV2::DecrementConnectedPlayerCount()
{
	if (CurrentPlayerCount > 0)
	{
		--CurrentPlayerCount;
	}
	UE_LOG(LogTemp, Log, TEXT("ConnectedPlayerCount Decremented: %d"), CurrentPlayerCount);

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (UCMGameInstance* CMGI = Cast<UCMGameInstance>(World->GetGameInstance()))
			{
				CMGI->SaveMainStageCounts(CurrentPlayerCount, DeathCount);
			}
		}
	}

	CheckAllPlayersDeadAndScheduleReturn();
}

void ACMGameStateMainStageV2::IncrementDeathCount()
{
	++DeathCount;
	UE_LOG(LogTemp, Log, TEXT("DeathCount Incremented: %d"), DeathCount);

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (UCMGameInstance* CMGI = Cast<UCMGameInstance>(World->GetGameInstance()))
			{
				CMGI->SaveMainStageCounts(CurrentPlayerCount, DeathCount);
			}
		}
	}

	CheckAllPlayersDeadAndScheduleReturn();
}

void ACMGameStateMainStageV2::DecrementDeathCount()
{
	if (DeathCount > 0)
	{
		--DeathCount;
	}
	UE_LOG(LogTemp, Log, TEXT("DecrementDeathCount: CurrentPlayerCount=%d, DeathCount=%d, HasAuthority=%d"), CurrentPlayerCount, DeathCount, HasAuthority());

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (UCMGameInstance* CMGI = Cast<UCMGameInstance>(World->GetGameInstance()))
			{
				CMGI->SaveMainStageCounts(CurrentPlayerCount, DeathCount);
			}
		}
	}

	CheckAllPlayersDeadAndScheduleReturn();
}

void ACMGameStateMainStageV2::HandleGameVictory()
{
	// 서버에서만 승리 처리
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleGameVictory called on non-authority"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HandleGameVictory: Broadcasting victory to all clients and scheduling return to lobby."));

	// 1) 모든 클라이언트에 승리 UI 표시
	Multicast_ShowVictoryUI();

	// 2) 패배 처리와 동일하게 일정 시간 후 로비로 복귀
	if (UWorld* World = GetWorld())
	{
		if (ACMGameModeMainStage* GM = World->GetAuthGameMode<ACMGameModeMainStage>())
		{
			FTimerHandle ReturnTimerHandle;
			World->GetTimerManager().SetTimer(
				ReturnTimerHandle,
				FTimerDelegate::CreateLambda([GM]()
				{
					UE_LOG(LogTemp, Log, TEXT("HandleGameVictory timer fired, calling GM->ReturnToExitMap"));
					GM->ReturnToExitMap();
				}),
				ReturnToLobbyDelay,
				false
			);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HandleGameVictory: AuthGameMode is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleGameVictory: World is null"));
	}
}

void ACMGameStateMainStageV2::CheckAllPlayersDeadAndScheduleReturn()
{
	UE_LOG(LogTemp, Log, TEXT("CheckAllPlayersDeadAndScheduleReturn called: CurrentPlayerCount=%d, DeathCount=%d, HasAuthority=%d"), CurrentPlayerCount, DeathCount, HasAuthority());
	// 모든 플레이어가 죽었는지 확인: 사망 수 == 접속 플레이어 수
	if (DeathCount == CurrentPlayerCount && CurrentPlayerCount > 0)
	{
		// 서버에서만 처리
		if (HasAuthority())
		{
			// 1) 모든 클라이언트에 패배 UI 표시를 멀티캐스트로 전파
			Multicast_ShowDefeatUI();
		}

		UE_LOG(LogTemp, Log, TEXT("All players dead. Requesting return to lobby after delay %.2f seconds."), ReturnToLobbyDelay);

		UWorld* World = GetWorld();
		if (World)
		{
			ACMGameModeMainStage* GM = World->GetAuthGameMode<ACMGameModeMainStage>();
			if (GM)
			{
				// 딜레이 후 로비로 돌아가도록 타이머 설정
				FTimerHandle ReturnTimerHandle;
				World->GetTimerManager().SetTimer(
					ReturnTimerHandle,
					FTimerDelegate::CreateLambda([GM]()
					{
						GM->ReturnToExitMap();
					}),
					ReturnToLobbyDelay,
					false
				);
			}
		}
	}
}

void ACMGameStateMainStageV2::Multicast_ShowDefeatUI_Implementation()
{
	// 서버 + 모든 클라이언트에서 실행됨
	if (UWorld* World = GetWorld())
	{
		UE_LOG(LogTemp, Log, TEXT("Multicast_ShowDefeatUI_Implementation: showing defeat UI on all local controllers in this world"));
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (ACMPlayerController* CMPC = Cast<ACMPlayerController>(It->Get()))
			{
				CMPC->ShowDefeatUI();
			}
		}
	}
}

void ACMGameStateMainStageV2::Multicast_ShowVictoryUI_Implementation()
{
	// 서버 + 모든 클라이언트에서 실행됨
	if (UWorld* World = GetWorld())
	{
		UE_LOG(LogTemp, Log, TEXT("Multicast_ShowVictoryUI_Implementation: showing victory UI on all local controllers in this world"));
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (ACMPlayerController* CMPC = Cast<ACMPlayerController>(It->Get()))
			{
				CMPC->ShowVictoryUI();
			}
		}
	}
}

void ACMGameStateMainStageV2::RegisterBoss(ACMEnemyCharacterBase* NewBoss)
{
	if (HasAuthority())
	{
		CurrentBoss = NewBoss;

		if (CurrentBoss)
		{
			if (OnBossSpawned.IsBound())
			{
				OnBossSpawned.Broadcast(CurrentBoss);
			}
		}
	}
}

void ACMGameStateMainStageV2::OnRep_CurrentBoss()
{
	if (CurrentBoss)
	{
		if (OnBossSpawned.IsBound())
		{
			OnBossSpawned.Broadcast(CurrentBoss);
		}

		UE_LOG(LogTemp, Log, TEXT("Client received Boss Info: %s"), *CurrentBoss->GetName());
	}
}
