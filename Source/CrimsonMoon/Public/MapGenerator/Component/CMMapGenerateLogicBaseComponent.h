// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMMapGenerateLogicBaseComponent.generated.h"


struct FCMRoomPosition;
class ACMRoom;
class UCMRoomDataDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMSONMOON_API UCMMapGenerateLogicBaseComponent : public UActorComponent
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	UCMMapGenerateLogicBaseComponent();

protected:
	virtual void BeginPlay() override;

	/* Custom Methods */
public:
	virtual void ExecuteMapGenerationLogic(int32 InSeed, UCMRoomDataDefinition* InMapData, TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& OutRoomMap);

protected:
	virtual void GenerateRoom(int32 InSeed, UCMRoomDataDefinition* InMapData, TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& OutRoomMap);
	virtual void PlaceSpecialRooms(int32 InSeed, UCMRoomDataDefinition* InMapData, TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& OutRoomMap);
	virtual void PlaceWallsAndEntrances(TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& OutRoomMap);

private:
	// GenerateRoom에서 사용하는 방향 인덱스 체계(0:Up, 1:Left, 2:Down, 3:Right)에 맞춘 오프셋
	int32 DirectionX[4] = { 0, -1,  0,  1}; // 상, 좌, 하, 우
	int32 DirectionY[4] = { 1,  0, -1,  0}; // 상, 좌, 하, 우
	
	// 방향 정의: 0:Up, 1:Left, 2:Down, 3:Right (상좌하우 순서로 고정)
	int32 DirIndexFromDelta(int32 DX, int32 DY);
	int32 OppositeDir(int32 Dir);
};