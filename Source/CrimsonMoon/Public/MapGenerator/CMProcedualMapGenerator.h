// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CMRoom.h"
#include "GameFramework/Actor.h"
#include "Interface/CMMapGenerate.h"
#include "CMProcedualMapGenerator.generated.h"

class UCMSpawnPositionSelectorBaseComponent;
class UCMMapGenerateLogicBaseComponent;

USTRUCT()
struct FCMRoomPosition
{
	GENERATED_BODY()

	int32 X;
	int32 Y;

	FORCEINLINE FCMRoomPosition() : X(0), Y(0) {}
	FORCEINLINE FCMRoomPosition(int32 InX, int32 InY) : X(InX), Y(InY) {}

	FORCEINLINE bool operator==(const FCMRoomPosition& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}
};

// 전역 해시 함수
FORCEINLINE uint32 GetTypeHash(const FCMRoomPosition& Pos)
{
	// 두 좌표 해시 결합
	return HashCombine(::GetTypeHash(Pos.X), ::GetTypeHash(Pos.Y));
}

USTRUCT()
struct FCMRoomNode
{
	GENERATED_BODY()
	
};

UCLASS()
class CRIMSONMOON_API ACMProcedualMapGenerator : public AActor, public ICMMapGenerate
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	ACMProcedualMapGenerator();

protected:
	virtual void BeginPlay() override;
	
	/* Custom Methods */
public:
	virtual void GenerateMap(int32 InSeed, UCMRoomDataDefinition* InMapData) override;
	virtual bool CheckAndAddRoomMapEntry(FCMRoomPosition RoomPosition, TObjectPtr<ACMRoom> Room);

	FORCEINLINE TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& GetRoomMap() { return RoomMap; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Generation")
	TObjectPtr<UCMMapGenerateLogicBaseComponent> MapGenerateLogicComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Generation")
	TObjectPtr<UCMSpawnPositionSelectorBaseComponent> SpawnPositionSelectorComponent;
	
private:
	UPROPERTY()
	TObjectPtr<UCMRoomDataDefinition> MapData;
	UPROPERTY()
	int32 Seed = -1;

	UPROPERTY()
	TMap<FCMRoomPosition, TObjectPtr<ACMRoom>> RoomMap;

};
