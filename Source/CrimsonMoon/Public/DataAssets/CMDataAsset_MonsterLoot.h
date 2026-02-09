// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CMDataAsset_MonsterLoot.generated.h"

class UCMDataAsset_ItemBase;

// 개별 아이템의 드랍 정보 구조체
USTRUCT(BlueprintType)
struct FMonsterLootItem
{
	GENERATED_BODY()

	// 떨어뜨릴 아이템 데이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UCMDataAsset_ItemBase> ItemToDrop;

	// 드랍 확률 (0.0 ~ 1.0, 예: 0.5 = 50%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 1.0f;

	// 최소/최대 수량 (예: 골드 10~100원)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint DropQuantityRange = FIntPoint(1, 1);
};

/**
 *  몬스터가 죽을 때 떨어뜨릴 아이템 목록
 */
UCLASS()
class CRIMSONMOON_API UCMDataAsset_MonsterLoot : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 드랍 가능한 아이템 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	TArray<FMonsterLootItem> LootItems;
};
