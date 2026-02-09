// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CMGameStateBase.h"
#include "MapGenerator/CMRoomDataDefinition.h"
#include "CMGameStateMainStage.generated.h"

class UCMRoomDataDefinition;
class ACMEnemyCharacterBase;

DECLARE_MULTICAST_DELEGATE(FOnGenerateMap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSpawned, ACMEnemyCharacterBase*, NewBoss);

UCLASS()
class CRIMSONMOON_API ACMGameStateMainStage : public ACMGameStateBase
{
	GENERATED_BODY()

public:
	ACMGameStateMainStage();

	FORCEINLINE int32 GetSeed() const { return Seed; }
	FORCEINLINE void SetSeed(int32 InSeed) { Seed = InSeed; }

	FOnGenerateMap OnGenerateMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Generation")
	FName MapDefinitionName;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Seed();

	UPROPERTY(ReplicatedUsing = OnRep_Seed, BlueprintReadOnly, Category = "Seed", meta = (AllowPrivate))
	int32 Seed = -1;

	// 에디터에서 여러 개의 룸 데이터(DataAsset)를 선택할 수 있도록 SoftObjectPtr 배열로 선언
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map")
	TArray<TSoftObjectPtr<UCMRoomDataDefinition>> RoomDataDefinitions;

	virtual void HandleLoadBegin() override;
	virtual void HandleGenerateMap();

private:
	void CreateMapGenerator();
	void RequestGenerateMap();

#pragma region Boss
public:
	FOnBossSpawned OnBossSpawned;
	
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void RegisterBoss(ACMEnemyCharacterBase* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	FORCEINLINE ACMEnemyCharacterBase* GetCurrentBoss() const { return CurrentBoss; }

private:
	TObjectPtr<ACMEnemyCharacterBase> CurrentBoss;
#pragma endregion
};
