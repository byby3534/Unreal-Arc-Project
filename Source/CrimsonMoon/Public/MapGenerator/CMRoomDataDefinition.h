// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "MapGenerator/Component/CMMapGenerateLogicBaseComponent.h" // 추가: 맵 생성 로직 컴포넌트
#include "MapGenerator/Component/CMSpawnPositionSelectorBaseComponent.h" // 추가: 스폰 위치 선택 컴포넌트
#include "CMRoomDataDefinition.generated.h"

class ACMRoom;

/**
 * 
 */
UCLASS(BlueprintType)
class CRIMSONMOON_API UCMRoomDataDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
	FName MapName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms", meta = (ClampMin = "1"))
	int32 Width;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms", meta = (ClampMin = "1"))
	int32 Height;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms", meta = (ClampMin = "1"))
	int32 MaxRoomCount; // 최대 방 개수
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rooms", meta = (ClampMin = "1"))
	int32 TreasureRoomCount; // 보물 방 개수
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rooms", meta = (ClampMin = "1"))
	int32 BossRoomCount; // 보스 방 개수
	

	// 에디터에서 지정 가능한 방 블루프린트(ACMRoom 파생) 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
	TArray<TSubclassOf<ACMRoom>> MonsterRoomClasses;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
	TArray<TSubclassOf<ACMRoom>> TreasureRoomClasses;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
	TArray<TSubclassOf<ACMRoom>> BossRoomClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	TSubclassOf<UCMMapGenerateLogicBaseComponent> MapGenerateLogicComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	TSubclassOf<UCMSpawnPositionSelectorBaseComponent> SpawnPositionSelectorComponentClass;
};
