// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Object/CMItemInstance.h"
#include "CMInventoryComponent.generated.h"

class UCMItemInstance;
class UCMDataAsset_ItemBase;

// QuickBar UI 갱신용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemChanged, UCMItemInstance*, ChangedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
// 대체 아이템을 찾았을 때 방송
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReplacementItemFound, const UCMDataAsset_ItemBase*, OriginalData, UCMItemInstance*, NewInstance);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CRIMSONMOON_API UCMInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCMInventoryComponent();
	void BeginPlay();

	// 아이템 추가 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(UCMDataAsset_ItemBase* NewItemData, int32 Amount = 1);

	// 아이템 지우는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UCMDataAsset_ItemBase* ItemToRemove, int32 Amount = 1);

	// 인스턴스를 직접 제거하는 함수 (장비 해제/버리기 등)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemInstance(UCMItemInstance* ItemInstance, int32 Amount = 1);
	
	// 특정 타입들 중 하나에 해당하고, 수량이 남은 첫 번째 아이템을 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UCMItemInstance* FindNextItemByTypes(const TArray<EConsumableType>& TargetTypes, const TArray<UCMItemInstance*>& IgnoreItems) const;
	
	// 목록 반환 타입 변경
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<UCMItemInstance*>& GetInventoryItems() const { return InventoryItems; }

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryItemChanged OnInventoryItemChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnReplacementItemFound OnReplacementItemFound;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DebugPrintInventory();

protected:
	// UObject 포인터 배열
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_InventoryItems)
	TArray<TObjectPtr<UCMItemInstance>> InventoryItems;

	// 빠른 검색을 위한 캐시용 맵 (Key: 데이터 에셋, Value: 인스턴스)
	UPROPERTY()
	TMap<UCMDataAsset_ItemBase*, UCMItemInstance*> ItemMapCache;

	// 인벤토리 최대 슬롯 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 MaxInventorySize = 20;

	// UObject 배열 복제
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
    
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_InventoryItems();

private:
	// 내부 헬퍼
	UCMItemInstance* FindItemByData(UCMDataAsset_ItemBase* ItemData) const;

	// 배열을 기반으로 맵을 다시 빌드하는 함수
	void RebuildItemMap();
};
