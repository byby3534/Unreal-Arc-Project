// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator/CMProcedualMapGenerator.h"

#include "MapGenerator/CMRoomDataDefinition.h"
#include "MapGenerator/Component/CMMapGenerateLogicBaseComponent.h"
#include "MapGenerator/Component/CMSpawnPositionSelectorBaseComponent.h"


// Sets default values
ACMProcedualMapGenerator::ACMProcedualMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	
}

void ACMProcedualMapGenerator::GenerateMap(int32 InSeed, UCMRoomDataDefinition* InMapData)
{
	if (!InMapData)
	{
		UE_LOG(LogTemp, Error, TEXT("GenerateMap: MapData is null"));
		return;
	}

	MapData = InMapData; // 포인터로 보관
	Seed = InSeed;

	// 맵 생성 로직 컴포넌트 클래스 동적 생성/교체
	if (MapData->MapGenerateLogicComponentClass)
	{
		UClass* DesiredClass = MapData->MapGenerateLogicComponentClass.Get();
		if (!MapGenerateLogicComponent || MapGenerateLogicComponent->GetClass() != DesiredClass)
		{
			if (MapGenerateLogicComponent)
			{
				MapGenerateLogicComponent->DestroyComponent();
				MapGenerateLogicComponent = nullptr;
			}
			MapGenerateLogicComponent = NewObject<UCMMapGenerateLogicBaseComponent>(this, DesiredClass, TEXT("MapGenerateLogicComponent"));
			if (MapGenerateLogicComponent)
			{
				MapGenerateLogicComponent->RegisterComponent();
			}
		}
	}
	else
	{
		if (MapGenerateLogicComponent)
		{
			MapGenerateLogicComponent->DestroyComponent();
			MapGenerateLogicComponent = nullptr;
		}
	}

	// 스폰 위치 선택 컴포넌트 클래스 동적 생성/교체
	if (MapData->SpawnPositionSelectorComponentClass)
	{
		UClass* DesiredClass = MapData->SpawnPositionSelectorComponentClass.Get();
		if (!SpawnPositionSelectorComponent || SpawnPositionSelectorComponent->GetClass() != DesiredClass)
		{
			if (SpawnPositionSelectorComponent)
			{
				SpawnPositionSelectorComponent->DestroyComponent();
				SpawnPositionSelectorComponent = nullptr;
			}
			SpawnPositionSelectorComponent = NewObject<UCMSpawnPositionSelectorBaseComponent>(this, DesiredClass, TEXT("SpawnPositionSelectorComponent"));
			if (SpawnPositionSelectorComponent)
			{
				SpawnPositionSelectorComponent->RegisterComponent();
			}
		}
	}
	else
	{
		if (SpawnPositionSelectorComponent)
		{
			SpawnPositionSelectorComponent->DestroyComponent();
			SpawnPositionSelectorComponent = nullptr;
		}
	}

	//TODO: Implement map generation logic here (use MapGenerateLogicComponent & SpawnPositionSelectorComponent)
	if (!MapGenerateLogicComponent)
	{
		MapGenerateLogicComponent = NewObject<UCMMapGenerateLogicBaseComponent>(this, UCMMapGenerateLogicBaseComponent::StaticClass(), TEXT("MapGenerateLogicComponent_Default"));
		if (MapGenerateLogicComponent)
		{
			MapGenerateLogicComponent->RegisterComponent();
		}
	}

	RoomMap.Empty();
	if (MapGenerateLogicComponent)
	{
		MapGenerateLogicComponent->ExecuteMapGenerationLogic(Seed, MapData, RoomMap);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GenerateMap: MapGenerateLogicComponent is null, cannot generate"));
	}
}

bool ACMProcedualMapGenerator::CheckAndAddRoomMapEntry(FCMRoomPosition RoomPosition, TObjectPtr<ACMRoom> Room)
{
	if (RoomMap.Contains(RoomPosition))
	{
		return false;
	}

	RoomMap.Add(RoomPosition, Room);
	return true;
}

void ACMProcedualMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}
