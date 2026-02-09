// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator/Component/CMMapGenerateLogicBaseComponent.h"
#include "MapGenerator/CMProcedualMapGenerator.h" // FCMRoomPosition, GetTypeHash
#include "MapGenerator/CMRoomDataDefinition.h"
#include "MapGenerator/CMRoom.h"
#include "Engine/World.h"
#include "Templates/UnrealTemplate.h" // Swap
#include "Containers/Queue.h"

UCMMapGenerateLogicBaseComponent::UCMMapGenerateLogicBaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCMMapGenerateLogicBaseComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCMMapGenerateLogicBaseComponent::ExecuteMapGenerationLogic(int32 InSeed, UCMRoomDataDefinition* InMapData, TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& OutRoomMap)
{
	GenerateRoom(InSeed, InMapData, OutRoomMap);
	PlaceWallsAndEntrances(OutRoomMap);
}

void UCMMapGenerateLogicBaseComponent::GenerateRoom(int32 InSeed, UCMRoomDataDefinition* InMapData, TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& OutRoomMap)
{
	// 방 생성 로직: 시드 기반 랜덤, 시작 좌표 {0,0}, 상/하/좌/우로 랜덤 확장, 보물/보스 방 배치, 마지막에 보스 방 배치
	OutRoomMap.Empty();

	if (!InMapData)
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteMapGenerationLogic: InMapData is null"));
		return;
	}

	// 시드 고정 랜덤 스트림
	FRandomStream RNG(InSeed);

	const int32 MaxRooms = FMath::Max(1, InMapData->MaxRoomCount);
	const int32 DesiredTreasure = FMath::Max(0, InMapData->TreasureRoomCount);
	const int32 DesiredBoss = FMath::Max(0, InMapData->BossRoomCount);

	// 맵 경계 계산(0,0 중심)
	const int32 HalfW = FMath::Max(0, InMapData->Width / 2);
	const int32 HalfH = FMath::Max(0, InMapData->Height / 2);
	const int32 MinX = -HalfW;
	const int32 MaxX = MinX + FMath::Max(1, InMapData->Width) - 1;
	const int32 MinY = -HalfH;
	const int32 MaxY = MinY + FMath::Max(1, InMapData->Height) - 1;

	const FCMRoomPosition DirOffset[4] = {
		FCMRoomPosition(0, 1),  // Up
		FCMRoomPosition(1, 0), // Left
		FCMRoomPosition(0, -1), // Down
		FCMRoomPosition(-1, 0)   // Right
	};

	// 그래프 자료구조
	TMap<FCMRoomPosition, TArray<FCMRoomPosition>> Adjacency; // 양방향 연결
	TMap<FCMRoomPosition, int32> ParentDirIndex; // 각 노드에서 부모가 위치한 방향 인덱스(0~3), 시작은 -1
	TArray<FCMRoomPosition> DFSStack;
	TArray<FCMRoomPosition> CreationOrder;

	auto IsOccupied = [&OutRoomMap](const FCMRoomPosition& Pos) -> bool
	{
		return OutRoomMap.Contains(Pos);
	};

	// 시작 노드 추가 {0,0}
	const FCMRoomPosition Start(0, 0);
	OutRoomMap.Add(Start, nullptr); // 스폰 전까지 null
	ParentDirIndex.Add(Start, -1);
	DFSStack.Add(Start);
	CreationOrder.Add(Start);

	// DFS로 상하좌우 랜덤 확장 (스패닝 트리 형태)
	while (OutRoomMap.Num() < MaxRooms && DFSStack.Num() > 0)
	{
		const FCMRoomPosition Curr = DFSStack.Last();

		TArray<int32> CandidateDirs;
		CandidateDirs.Reserve(4);
		for (int32 d = 0; d < 4; ++d)
		{
			const FCMRoomPosition& Off = DirOffset[d];
			FCMRoomPosition Next(Curr.X + Off.X, Curr.Y + Off.Y);
			if (!IsOccupied(Next)
				&& Next.X >= MinX && Next.X <= MaxX
				&& Next.Y >= MinY && Next.Y <= MaxY)
			{
				CandidateDirs.Add(d);
			}
		}

		if (CandidateDirs.Num() > 0)
		{
			const int32 ChoiceIdx = RNG.RandRange(0, CandidateDirs.Num() - 1);
			const int32 Dir = CandidateDirs[ChoiceIdx];
			const FCMRoomPosition& Off = DirOffset[Dir];
			const FCMRoomPosition Next(Curr.X + Off.X, Curr.Y + Off.Y);

			// 연결 기록
			Adjacency.FindOrAdd(Curr).Add(Next);
			Adjacency.FindOrAdd(Next).Add(Curr);
			ParentDirIndex.Add(Next, OppositeDir(Dir)); // 자식 노드에서 부모가 있는 방향

			// 맵에 방 추가 (스폰은 나중에)
			OutRoomMap.Add(Next, nullptr);
			DFSStack.Add(Next);
			CreationOrder.Add(Next);
		}
		else
		{
			DFSStack.Pop(); // 막다른 길이면 백트래킹
		}
	}

	// 거리 계산(BFS)로 시작점에서의 거리 파악
	TMap<FCMRoomPosition, int32> Distance;
	Distance.Add(Start, 0);
	TQueue<FCMRoomPosition> Q;
	Q.Enqueue(Start);
	while (!Q.IsEmpty())
	{
		FCMRoomPosition Curr;
		Q.Dequeue(Curr);
		const TArray<FCMRoomPosition>* NeighPtr = Adjacency.Find(Curr);
		if (!NeighPtr) continue;
		for (const FCMRoomPosition& N : *NeighPtr)
		{
			if (!Distance.Contains(N))
			{
				Distance.Add(N, Distance[Curr] + 1);
				Q.Enqueue(N);
			}
		}
	}

	// 리프 노드(차수 1, 시작 제외)를 거리 내림차순으로 정렬
	TArray<FCMRoomPosition> Leaves;
	Leaves.Reserve(Adjacency.Num());
	for (const TPair<FCMRoomPosition, TArray<FCMRoomPosition>>& Pair : Adjacency)
	{
		const FCMRoomPosition& Pos = Pair.Key;
		const int32 Degree = Pair.Value.Num();
		if (Degree == 1 && !(Pos.X == Start.X && Pos.Y == Start.Y))
		{
			Leaves.Add(Pos);
		}
	}
	Leaves.Sort([&Distance](const FCMRoomPosition& A, const FCMRoomPosition& B)
	{
		const int32 DA = Distance.Contains(A) ? Distance[A] : MAX_int32;
		const int32 DB = Distance.Contains(B) ? Distance[B] : MAX_int32;
		return DA > DB; // 먼 순서대로
	});

	// 보스/보물 방 배치 결정
	TSet<FCMRoomPosition> BossPositions;
	TSet<FCMRoomPosition> TreasurePositions;

	const int32 BossToPlace = FMath::Clamp(DesiredBoss, 0, Leaves.Num());
	for (int32 i = 0; i < BossToPlace; ++i)
	{
		BossPositions.Add(Leaves[i]);
	}

	// 보물 방은 시작/보스 제외에서 랜덤 선택
	TArray<FCMRoomPosition> CandidatesForTreasure;
	CandidatesForTreasure.Reserve(OutRoomMap.Num());
	for (const TPair<FCMRoomPosition, TObjectPtr<ACMRoom>>& Pair : OutRoomMap)
	{
		const FCMRoomPosition& Pos = Pair.Key;
		if ((Pos.X == Start.X && Pos.Y == Start.Y)) continue; // 시작 제외
		if (BossPositions.Contains(Pos)) continue; // 보스 제외
		CandidatesForTreasure.Add(Pos);
	}
	// 셔플
	for (int32 i = CandidatesForTreasure.Num() - 1; i > 0; --i)
	{
		int32 j = RNG.RandRange(0, i);
		Swap(CandidatesForTreasure[i], CandidatesForTreasure[j]);
	}
	const int32 TreasureToPlace = FMath::Clamp(DesiredTreasure, 0, CandidatesForTreasure.Num());
	for (int32 i = 0; i < TreasureToPlace; ++i)
	{
		TreasurePositions.Add(CandidatesForTreasure[i]);
	}

	// 방 액터 스폰
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteMapGenerationLogic: World is null, cannot spawn rooms"));
		return;
	}

	// 스폰 파라미터
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 방 클래스 선택 헬퍼
	auto ChooseClass = [&RNG](const TArray<TSubclassOf<ACMRoom>>& Classes) -> TSubclassOf<ACMRoom>
	{
		if (Classes.Num() <= 0) return nullptr;
		const int32 Index = RNG.RandRange(0, Classes.Num() - 1);
		return Classes[Index];
	};

	// 대표 간격 계산: 모든 Room 클래스 CDO의 최대 폭/높이 사용, 너무 작으면 샘플 스폰으로 재측정 후 폴백
	const float MinValidSpacing = 500.f; // 임계값
	float SpacingX = 0.f;
	float SpacingY = 0.f;

	auto AccumulateMaxSize = [&](const TArray<TSubclassOf<ACMRoom>>& Classes)
	{
		for (const TSubclassOf<ACMRoom>& C : Classes)
		{
			if (!C) continue;
			const ACMRoom* DefaultRoomCDO = C->GetDefaultObject<ACMRoom>();
			if (!DefaultRoomCDO) continue;
			SpacingX = FMath::Max(SpacingX, DefaultRoomCDO->GetRoomWidth());
			SpacingY = FMath::Max(SpacingY, DefaultRoomCDO->GetRoomHeight());
		}
	};

	AccumulateMaxSize(InMapData->MonsterRoomClasses);
	AccumulateMaxSize(InMapData->TreasureRoomClasses);
	AccumulateMaxSize(InMapData->BossRoomClasses);

	bool bNeedSampleSpawn = (SpacingX < MinValidSpacing || SpacingY < MinValidSpacing);
	if (bNeedSampleSpawn)
	{
		// 사용 가능한 아무 클래스 하나 선택
		TSubclassOf<ACMRoom> SampleClass = nullptr;
		if (InMapData->MonsterRoomClasses.Num() > 0) SampleClass = InMapData->MonsterRoomClasses[0];
		else if (InMapData->TreasureRoomClasses.Num() > 0) SampleClass = InMapData->TreasureRoomClasses[0];
		else if (InMapData->BossRoomClasses.Num() > 0) SampleClass = InMapData->BossRoomClasses[0];

		if (SampleClass)
		{
			ACMRoom* SampleRoom = World->SpawnActor<ACMRoom>(SampleClass, FVector(0,0,-100000.f), FRotator::ZeroRotator, SpawnParams); // 화면 밖 임시 스폰
			if (SampleRoom)
			{
				float SampleW = SampleRoom->GetRoomWidth();
				float SampleH = SampleRoom->GetRoomHeight();
				SpacingX = FMath::Max(SpacingX, SampleW);
				SpacingY = FMath::Max(SpacingY, SampleH);
				SampleRoom->Destroy();
			}
		}
	}

	if (SpacingX < MinValidSpacing) SpacingX = 4000.f;
	if (SpacingY < MinValidSpacing) SpacingY = 4000.f;

	// 먼저 모든 방을 스폰하고 맵에 포인터 갱신
	for (const FCMRoomPosition& Pos : CreationOrder)
	{
		bool bIsBoss = BossPositions.Contains(Pos);
		bool bIsTreasure = !bIsBoss && TreasurePositions.Contains(Pos);

		TSubclassOf<ACMRoom> ClassToSpawn = nullptr;
		if (bIsBoss)
		{
			ClassToSpawn = ChooseClass(InMapData->BossRoomClasses);
			if (!ClassToSpawn)
			{
				UE_LOG(LogTemp, Warning, TEXT("No BossRoomClasses available, falling back to MonsterRoomClasses"));
				ClassToSpawn = ChooseClass(InMapData->MonsterRoomClasses);
			}
		}
		else if (bIsTreasure)
		{
			ClassToSpawn = ChooseClass(InMapData->TreasureRoomClasses);
			if (!ClassToSpawn)
			{
				UE_LOG(LogTemp, Warning, TEXT("No TreasureRoomClasses available, falling back to MonsterRoomClasses"));
				ClassToSpawn = ChooseClass(InMapData->MonsterRoomClasses);
			}
		}
		else
		{
			ClassToSpawn = ChooseClass(InMapData->MonsterRoomClasses);
		}

		if (!ClassToSpawn)
		{
			UE_LOG(LogTemp, Error, TEXT("No room class available to spawn at (%d,%d)"), Pos.X, Pos.Y);
			continue;
		}

		const FVector Location(Pos.X * SpacingX, Pos.Y * SpacingY, 0.f);
		const FRotator Rotation = FRotator::ZeroRotator;
		ACMRoom* Spawned = World->SpawnActor<ACMRoom>(ClassToSpawn, Location, Rotation, SpawnParams);
		if (!Spawned)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn room at (%d,%d)"), Pos.X, Pos.Y);
			continue;
		}

		// 맵에 포인터 갱신
		OutRoomMap.FindOrAdd(Pos) = Spawned;
	}
	
	// 연결 설정 및 부모 인덱스 설정
	for (const TPair<FCMRoomPosition, TObjectPtr<ACMRoom>>& Pair : OutRoomMap)
	{
		const FCMRoomPosition& Pos = Pair.Key;
		ACMRoom* Room = Pair.Value.Get();
		if (!Room)
		{
			continue;
		}

		// 네 방향(0:Up,1:Left,2:Down,3:Right)에 대해 인접 좌표 계산
		for (int32 Dir = 0; Dir < 4; ++Dir)
		{
			const int32 NX = Pos.X + DirectionX[Dir];
			const int32 NY = Pos.Y + DirectionY[Dir];
			FCMRoomPosition NeighborPos(NX, NY);

			if (OutRoomMap.Contains(NeighborPos))
			{
				TObjectPtr<ACMRoom> NeighborRoom = OutRoomMap[NeighborPos];
				Room->SetConnectedRoom(Dir, NeighborRoom);
				NeighborRoom->SetConnectedRoom(OppositeDir(Dir), Room);
			}
		}

		if (const int32* ParentDir = ParentDirIndex.Find(Pos))
		{
			Room->SetParentRoomIndex(*ParentDir);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Map generation complete: Rooms=%d, Boss=%d, Treasure=%d"), OutRoomMap.Num(), BossPositions.Num(), TreasurePositions.Num());
}

void UCMMapGenerateLogicBaseComponent::PlaceSpecialRooms(int32 InSeed, UCMRoomDataDefinition* InMapData, TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& OutRoomMap) {}

void UCMMapGenerateLogicBaseComponent::PlaceWallsAndEntrances(TMap<FCMRoomPosition, TObjectPtr<ACMRoom>>& OutRoomMap)
{
	ENetMode NetMode = GetWorld() ? GetWorld()->GetNetMode() : NM_Standalone;
	for (const TPair<FCMRoomPosition, TObjectPtr<ACMRoom>>& Pair : OutRoomMap)
	{
		if (ACMRoom* Room = Pair.Value.Get())
		{
			for (int32 i = 0; i < 4; ++i)
			{
				FCMRoomPosition CurrentPos( Pair.Key.X + DirectionX[i], Pair.Key.Y + DirectionY[i] );
				if (OutRoomMap.Contains(CurrentPos))
				{
					if (NetMode != NM_Client)
					{
						Room->ExecuteSpawnRoom(i, true); // Entrance
					}
				}
				else
				{
					Room->ExecuteSpawnRoom(i, false); // Wall
				}
			}
		}
	}
}

int32 UCMMapGenerateLogicBaseComponent::DirIndexFromDelta(int32 DX, int32 DY)
{
	if (DY == 1 && DX == 0)
	{
		return 0;	// Up
	}
	if (DX == -1 && DY == 0)
	{
		return 1;   // Left
	}
	if (DY == -1 && DX == 0)
	{
		return 2;   // Down
	}
	if (DX == 1 && DY == 0) 
	{
		return 3;	// Right
	}
	return -1;	
}

int32 UCMMapGenerateLogicBaseComponent::OppositeDir(int32 Dir)
{
	// 상좌하우 인덱스에 대한 반대 방향 (Up<->Down, Left<->Right)
	switch (Dir)
	{
		case 0: return 2; // Up -> Down
		case 1: return 3; // Left -> Right
		case 2: return 0; // Down -> Up
		case 3: return 1; // Right -> Left
		default: return -1;
	}
}
