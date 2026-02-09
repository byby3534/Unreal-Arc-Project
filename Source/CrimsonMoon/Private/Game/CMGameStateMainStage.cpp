// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CMGameStateMainStage.h"
#include "Net/UnrealNetwork.h"
#include "MapGenerator/CMProcedualMapGenerator.h"

ACMGameStateMainStage::ACMGameStateMainStage()
{
	bReplicates = true;
	SyncedComponentMap.Add(FName("Seed"), 0);
}

void ACMGameStateMainStage::BeginPlay()
{
	if (HasAuthority())
	{
		Seed = FMath::RandRange(0, 1000000);
		NotifyComponentSynced(FName("Seed"));
	}

	//OnGenerateMap.AddUObject(this, &ACMGameStateMainStage::CreateMapGenerator);
	//OnGenerateMap.AddUObject(this, &ACMGameStateMainStage::RequestGenerateMap);
	
	Super::BeginPlay();
}

void ACMGameStateMainStage::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACMGameStateMainStage, Seed);
}

void ACMGameStateMainStage::OnRep_Seed()
{
	UE_LOG(LogTemp, Log, TEXT("Seed replicated: %d"), Seed);
	NotifyComponentSynced(FName("Seed"));
}

void ACMGameStateMainStage::HandleLoadBegin()
{
	UE_LOG(LogTemp, Log, TEXT("[Load] HandleLoadBegin -> Broadcast"));
	OnLoadBegin.Broadcast();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACMGameStateMainStage::HandleGenerateMap));
	}
}

void ACMGameStateMainStage::HandleGenerateMap()
{
	UE_LOG(LogTemp, Log, TEXT("[Load] HandleGenerateMap -> Broadcast"));
	OnGenerateMap.Broadcast();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACMGameStateMainStage::HandleLoadUI));
	}
}

void ACMGameStateMainStage::CreateMapGenerator()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateMapGenerator: World is null"));
		return;
	}

	if (RoomDataDefinitions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateMapGenerator: RoomDataDefinitions is empty"));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACMProcedualMapGenerator* Generator = World->SpawnActor<ACMProcedualMapGenerator>(ACMProcedualMapGenerator::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	UCMRoomDataDefinition* SelectedMapDef = nullptr;
	
	int32 Index = 0;
	for (const TSoftObjectPtr<UCMRoomDataDefinition>& SoftDef : RoomDataDefinitions)
	{
		UCMRoomDataDefinition* MapDef = SoftDef.IsValid() ? SoftDef.Get() : SoftDef.LoadSynchronous();
		if (!MapDef)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateMapGenerator: Failed to load RoomDataDefinition at index %d"), Index);
			++Index;
			continue;
		}
		
		UE_LOG(LogTemp, Log, TEXT("CreateMapGenerator: Loaded definition %d - %s"), Index, *MapDef->MapName.ToString());

		// Generator에 설정된 MapDefinitionName과 동일한 맵을 선택
		if (Generator && MapDef->MapName == MapDefinitionName)
		{
			SelectedMapDef = MapDef;
			UE_LOG(LogTemp, Log, TEXT("CreateMapGenerator: Selected definition '%s'"), *MapDef->MapName.ToString());
			break;
		}

		++Index;
	}
	
	// 시드와 데이터로 즉시 맵 생성 -> 내부에서 컴포넌트 생성/등록
	if (Generator)
	{
		Generator->GenerateMap(Seed, SelectedMapDef);
	}
}

void ACMGameStateMainStage::RequestGenerateMap()
{
	if (HasAuthority())
	{
		CreateMapGenerator();
	}
}

void ACMGameStateMainStage::RegisterBoss(ACMEnemyCharacterBase* NewBoss)
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
