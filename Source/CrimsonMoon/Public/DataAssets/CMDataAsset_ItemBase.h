// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/Enums/CMItemTier.h"
#include "GameplayTagContainer.h"
#include "CMDataAsset_ItemBase.generated.h"

/**
 * 아이템 베이스
 */
UCLASS()
class CRIMSONMOON_API UCMDataAsset_ItemBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 아이템 ID 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Common")
	FName ItemID;

	// 아이템 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
	FText ItemName;

	// 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
	TSoftObjectPtr<UTexture2D> ItemIcon;

	// 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI", meta = (MultiLine = "true"))
	FText ItemDescription;

	// 등급
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Common")
	ECMItemTier ItemTier;
    
	// 최대 스택 개수 (장비는 1, 소모품은 9등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Common")
	int32 MaxStackSize = 1;

	// 월드에 떨어져 있을 때 보여줄 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Pickup")
	TObjectPtr<UStaticMesh> PickupMesh;

	// 월드에 떨어져 있을 때 메쉬 크기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Pickup")
	FVector PickupMeshScale = FVector(1.0f);
};
