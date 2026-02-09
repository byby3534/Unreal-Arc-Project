// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CMNpcWorldSubsystem.generated.h"

class ACMNpcBase;

USTRUCT(BlueprintType)
struct FCMNpcCacheEntry
{
	GENERATED_BODY()

public:
	// 고유 식별용 ID (예: DataTable RowName, GameplayTag, 커스텀 ID 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FName NpcId = NAME_None;

	// 실제 월드에 존재하는 NPC 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	TWeakObjectPtr<ACMNpcBase> NpcActor;
};

UCLASS()
class CRIMSONMOON_API UCMNpcWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// NPC 캐시 등록 함수
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void RegisterNpc(const FName& NpcId, ACMNpcBase* NpcActor);

	// NPC 캐시 해제 함수
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void UnregisterNpc(const FName& NpcId);

	// ID로 NPC 액터를 조회하는 함수 (있으면 반환, 없으면 nullptr)
	UFUNCTION(BlueprintCallable, Category = "NPC")
	ACMNpcBase* GetNpcById(const FName& NpcId) const;

	// 전체 캐시 조회용 Getter
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void GetAllNpcEntries(TArray<FCMNpcCacheEntry>& OutEntries) const;

private:
	// ID 기반 NPC 캐시 맵
	UPROPERTY()
	TMap<FName, FCMNpcCacheEntry> NpcCacheMap;
};
