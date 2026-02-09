// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator/CMRoom.h"

#include "MapGenerator/Component/CMRoomBoundsBox.h"
#include "Engine/World.h"
#include "Engine/LevelStreamingDynamic.h"


// Sets default values
ACMRoom::ACMRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	RoomBoundsBox = CreateDefaultSubobject<UCMRoomBoundsBox>(TEXT("RoomBoundsBox"));
	if (RoomBoundsBox)
	{
		RoomBoundsBox->SetupAttachment(RootComponent);
	}

	NorthEntrancePoint = CreateDefaultSubobject<USceneComponent>(TEXT("NorthEntrancePoint"));
	SouthEntrancePoint = CreateDefaultSubobject<USceneComponent>(TEXT("SouthEntrancePoint"));
	EastEntrancePoint = CreateDefaultSubobject<USceneComponent>(TEXT("EastEntrancePoint"));
	WestEntrancePoint = CreateDefaultSubobject<USceneComponent>(TEXT("WestEntrancePoint"));
	if (NorthEntrancePoint)
	{
		NorthEntrancePoint->SetupAttachment(RootComponent);
	}
	if (SouthEntrancePoint)
	{
		SouthEntrancePoint->SetupAttachment(RootComponent);
	}
	if (EastEntrancePoint)
	{
		EastEntrancePoint->SetupAttachment(RootComponent);
	}
	if (WestEntrancePoint)
	{
		WestEntrancePoint->SetupAttachment(RootComponent);
	}
}

// Called when the game starts or when spawned
void ACMRoom::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACMRoom::ExecuteSpawnRoom(int32 DirectionIndex, bool bIsEntrance)
{
	SpawnBorderElement(DirectionIndex,  bIsEntrance);
}

AActor* ACMRoom::SpawnBorderElement(int32 DirectionIndex, bool bIsEntrance)
{
	if (DirectionIndex < 0 || DirectionIndex >= 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnBorderElement: Invalid DirectionIndex %d in %s"), DirectionIndex, *GetName());
		return nullptr;
	}

	if (!RoomBoundsBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnBorderElement: RoomBoundsBox is null in %s"), *GetName());
		return nullptr;
	}

	// Border 중복 스폰 방지
	if (RoomBorderActors[DirectionIndex])
	{
		return RoomBorderActors[DirectionIndex];
	}

	TSubclassOf<AActor> ClassToSpawn = bIsEntrance ? RoomEntranceClass : RoomWallClass;
	if (!ClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnBorderElement: Missing %s class in %s"), bIsEntrance ? TEXT("Entrance") : TEXT("Wall"), *GetName());
		return nullptr;
	}


	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	USceneComponent* EntrancePoint = nullptr;
	switch (DirectionIndex)
	{
		case 0: EntrancePoint = NorthEntrancePoint; break; // Up
		case 1: EntrancePoint = WestEntrancePoint; break; // Left
		case 2: EntrancePoint = SouthEntrancePoint; break; // Down
		case 3: EntrancePoint = EastEntrancePoint; break; // Right
		default: break;
	}

	if (!EntrancePoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnBorderElement: EntrancePoint is null for DirectionIndex %d in %s"), DirectionIndex, *GetName());
		return nullptr;
	}

	const FVector SpawnLocation = EntrancePoint->GetComponentLocation();
	const FRotator SpawnRotation = EntrancePoint->GetComponentRotation();
	FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ClassToSpawn, SpawnTransform, SpawnParams);
	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnBorderElement: Failed to spawn actor for DirectionIndex %d in %s"), DirectionIndex, *GetName());
		return nullptr;
	}

	// 연결된 룸에도 동일한 액터 포인터 설정
	RoomBorderActors[DirectionIndex] = SpawnedActor;
	if (ConnectedRooms[DirectionIndex]) // 반대 방향 인덱스
	{
		ConnectedRooms[DirectionIndex]->RoomBorderActors[(DirectionIndex + 2) % 4] = SpawnedActor;
	}
	

	// 성공 로그 (룸 위치 + 방향 인덱스 + 타입)
	const FVector RoomLoc = GetActorLocation();
	UE_LOG(LogTemp, Log, TEXT("SpawnBorderElement 성공: Room=%s RoomLoc=(%.1f, %.1f, %.1f) DirectionIndex=%d Type=%s"),
		*GetName(), RoomLoc.X, RoomLoc.Y, RoomLoc.Z, DirectionIndex, bIsEntrance ? TEXT("Entrance") : TEXT("Wall"));

	return SpawnedActor;
}