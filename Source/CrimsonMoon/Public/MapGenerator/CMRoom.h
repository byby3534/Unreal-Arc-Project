// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapGenerator/Component/CMRoomBoundsBox.h"
#include "CMRoom.generated.h"

UCLASS()
class CRIMSONMOON_API ACMRoom : public AActor
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	ACMRoom();

protected:
	virtual void BeginPlay() override;

	/* Custom Methods */
public:
	FORCEINLINE void SetParentRoomIndex(int32 InParentRoomIndex) { ParentRoomIndex = InParentRoomIndex; }
	FORCEINLINE ACMRoom* GetParentRoom() { return ConnectedRooms[ParentRoomIndex]; }
	FORCEINLINE TObjectPtr<ACMRoom>* GetConnectedRooms() { return ConnectedRooms; }
	FORCEINLINE void SetConnectedRoom(int32 DirectionIndex, TObjectPtr<ACMRoom> ConnectedRoom)
	{
		if (DirectionIndex >= 0 && DirectionIndex < 4)
		{
			ConnectedRooms[DirectionIndex] = ConnectedRoom;
		}
	}
	// 방의 월드 기준 가로/세로 길이(박스 익스텐트*2)를 반환
	FORCEINLINE float GetRoomWidth() const { return RoomBoundsBox ? RoomBoundsBox->GetScaledBoxExtent().X * 2.f : 0.f; }
	FORCEINLINE float GetRoomHeight() const { return RoomBoundsBox ? RoomBoundsBox->GetScaledBoxExtent().Y * 2.f : 0.f; }
	FORCEINLINE AActor* GetEntranceActor(int32 DirectionIndex) const
	{
		if (DirectionIndex >= 0 && DirectionIndex < 4)
		{
			return RoomBorderActors[DirectionIndex];
		}
		return nullptr;
	}
	
	// RoomBoundsBox의 가장자리에 Entrance 또는 Wall 액터를 스폰합니다.
	// DirectionIndex: 0=상(Up), 1=좌(Left), 2=하(Down), 3=우(Right)
	// bIsEntrance: true면 EntranceClass, false면 WallClass 사용
	void ExecuteSpawnRoom(int32 DirectionIndex, bool bIsEntrance);
	AActor* SpawnBorderElement(int32 DirectionIndex, bool bIsEntrance);

protected:
	// 레벨 스트리밍용 서브 레벨 이름 (에디터에서 설정)
	// 이 방이 활성화/비활성화될 때 매니저에서 해당 레벨을 Stream In/Out 하는 데 사용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Streaming")
	FName StreamingLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TObjectPtr<UCMRoomBoundsBox> RoomBoundsBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TObjectPtr<USceneComponent> NorthEntrancePoint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TObjectPtr<USceneComponent> SouthEntrancePoint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TObjectPtr<USceneComponent> EastEntrancePoint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TObjectPtr<USceneComponent> WestEntrancePoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TSubclassOf<AActor> RoomEntranceClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TSubclassOf<AActor> RoomWallClass;
	
private:
	UPROPERTY()
	int32 ParentRoomIndex;
	UPROPERTY()
	TObjectPtr<ACMRoom> ConnectedRooms[4]; // 상, 좌, 하, 우 (0=Up,1=Left,2=Down,3=Right)
	UPROPERTY()
	TObjectPtr<AActor> RoomBorderActors[4]; // 상, 좌, 하, 우 (0=Up,1=Left,2=Down,3=Right)

};