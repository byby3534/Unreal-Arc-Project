// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CMInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "DataAssets/Weapon/CMDataAsset_WeaponData.h"
#include "Engine/ActorChannel.h"
#include "Engine/Engine.h"
#include "Items/Object/CMItemInstance.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "DataAssets/Consumable/CMDataAsset_ConsumableData.h"

UCMInventoryComponent::UCMInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 맵 초기화
	RebuildItemMap();
}

void UCMInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UCMInventoryComponent, InventoryItems, COND_OwnerOnly);
}

UCMItemInstance* UCMInventoryComponent::FindNextItemByTypes(const TArray<EConsumableType>& TargetTypes, const TArray<UCMItemInstance*>& IgnoreItems) const
{
	// 인벤토리 순회
	for (UCMItemInstance* Item : InventoryItems)
	{
		// 유효성 및 수량 체크
		if (!Item || !IsValid(Item) || Item->Quantity <= 0)
		{
			continue;
		}
		
		if (IgnoreItems.Contains(Item))
		{
			continue;
		}

		// 데이터 에셋 타입 체크
		if (const UCMDataAsset_ConsumableData* Data = Cast<UCMDataAsset_ConsumableData>(Item->ItemData))
		{
			if (TargetTypes.Contains(Data->ConsumableType))
			{
				return Item;
			}
		}
	}

	return nullptr;
}

// UObject 배열 안에 있는 객체들을 클라이언트로 복제시키는 함수
bool UCMInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    // 배열을 순회하며 유효한 아이템 객체들을 복제 채널에 태움
    for (UCMItemInstance* Item : InventoryItems)
    {
        if (Item && IsValid(Item))
        {
            bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
        }
    }

    return bWroteSomething;
}

int32 UCMInventoryComponent::AddItem(UCMDataAsset_ItemBase* NewItemData, int32 Amount)
{
	// 유효성 검사
	if (!GetOwner()->HasAuthority() || !NewItemData || Amount <= 0)
	{
		return Amount;
	}
	
	// [기존 스택에 합치기]
	if (NewItemData->MaxStackSize > 1)
	{
		if (UCMItemInstance* ExistingItem = FindItemByData(NewItemData))
		{
			for (int32 i = 0; i < InventoryItems.Num(); ++i)
			{
				UCMItemInstance* Item = InventoryItems[i];
				
				if (Item && Item->ItemData == NewItemData)
				{
					// 이 슬롯에 더 들어갈 수 있는 공간 계산
					int32 SpaceRemaining = NewItemData->MaxStackSize - Item->Quantity;

					if (SpaceRemaining > 0)
					{
						int32 AmountToAdd = FMath::Min(SpaceRemaining, Amount);
						Item->Quantity += AmountToAdd;
						Amount -= AmountToAdd;

						// 변경 알림
						if (OnInventoryItemChanged.IsBound())
						{
							OnInventoryItemChanged.Broadcast(Item);
						}
						
						// 서버측 UI 갱신을 위해 수동 호출
						Item->OnRep_Quantity();

						if (Amount <= 0)
						{
							return 0;
						}
					}
				}
			}
		}
	}
	
	// [새 슬롯 생성]
	bool bNewItemCreated = false;
	
	while (Amount > 0)
	{
		// 현재 인벤토리 칸 수가 최대치 이상인지 확인
		if (InventoryItems.Num() >= MaxInventorySize)
		{
			// 꽉 찼으므로 더 이상 넣지 못함
			UE_LOG(LogTemp, Warning, TEXT("인벤토리가 가득 찼습니다! 남은 수량: %d"), Amount);
			
			// UI 전체 갱신
			OnRep_InventoryItems();
			
			return Amount; 
		}

		// 이번 슬롯에 넣을 수량 결정
		int32 AmountToAdd = FMath::Min(Amount, NewItemData->MaxStackSize);

		// 새 인스턴스 생성
		UCMItemInstance* NewItem = NewObject<UCMItemInstance>(this, UCMItemInstance::StaticClass());
		NewItem->ItemData = NewItemData;
		NewItem->Quantity = AmountToAdd;
		
		InventoryItems.Add(NewItem);

		// 만약 이 아이템이 맵에 없다면 등록
		if (!ItemMapCache.Contains(NewItemData))
		{
			ItemMapCache.Add(NewItemData, NewItem);
		}
		
		Amount -= AmountToAdd;
		bNewItemCreated = true;

		// 알림
		if (OnInventoryItemChanged.IsBound())
		{
			OnInventoryItemChanged.Broadcast(NewItem);
		}
		
		// 로그
		UE_LOG(LogTemp, Log, TEXT("새 슬롯 생성: %s (수량: %d)"), *NewItemData->GetName(), AmountToAdd);
	}
	
	if (bNewItemCreated)
	{
		OnRep_InventoryItems();
	}
	
	return 0;
}

bool UCMInventoryComponent::RemoveItem(UCMDataAsset_ItemBase* ItemToRemove, int32 Amount)
{
    // 데이터 에셋 기준으로 찾아서 제거
    UCMItemInstance* TargetItem = FindItemByData(ItemToRemove);
    return RemoveItemInstance(TargetItem, Amount);
}

bool UCMInventoryComponent::RemoveItemInstance(UCMItemInstance* ItemInstance, int32 Amount)
{
	if (!GetOwner()->HasAuthority() || !ItemInstance || Amount <= 0)
	{
		return false;
	}

    // 인벤토리에 실제로 들어있는지 확인
    if (!InventoryItems.Contains(ItemInstance))
    {
        return false;
    }

    // 수량 차감
    ItemInstance->Quantity -= Amount;

    // 변경 알림
    if (OnInventoryItemChanged.IsBound())
    {
        OnInventoryItemChanged.Broadcast(ItemInstance);
    }
    
    // 서버측 UI 갱신
    if (GetOwner()->GetNetMode() == NM_ListenServer)
    {
	    // 리슨 서버인 경우 직접 호출
	    ItemInstance->OnRep_Quantity();
    }
    else
    {
	    // 전용 서버인 경우 클라이언트로 복제될 때 OnRep_Quantity가 호출됨
    }

    // 수량이 0 이하가 되면 배열에서 제거
    if (ItemInstance->Quantity <= 0)
    {
        InventoryItems.Remove(ItemInstance);

    	// [맵 갱신]
    	RebuildItemMap();
        OnRep_InventoryItems();
    }

	return true;
}

UCMItemInstance* UCMInventoryComponent::FindItemByData(UCMDataAsset_ItemBase* ItemData) const
{
	if (!ItemData)
	{
		return nullptr;
	}

	// 맵에서 바로 검색 (포인터 반환)
	if (UCMItemInstance* const* FoundItem = ItemMapCache.Find(ItemData))
	{
		return *FoundItem;
	}

	return nullptr;
}

void UCMInventoryComponent::RebuildItemMap()
{
	ItemMapCache.Empty();

	for (UCMItemInstance* Item : InventoryItems)
	{
		if (Item && Item->ItemData)
		{
			// 맵에 없는 경우에만 추가
			if (!ItemMapCache.Contains(Item->ItemData))
			{
				ItemMapCache.Add(Item->ItemData, Item);
			}
		}
	}
}

void UCMInventoryComponent::OnRep_InventoryItems()
{
	RebuildItemMap();
	OnInventoryUpdated.Broadcast();
}

void UCMInventoryComponent::DebugPrintInventory()
{
	FString HeaderMsg = FString::Printf(TEXT("--- 인벤토리 확인 (총 %d 슬롯) ---"), InventoryItems.Num());
    
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, HeaderMsg);
		UE_LOG(LogTemp, Log, TEXT("%s"), *HeaderMsg);
	}

	for (int32 i = 0; i < InventoryItems.Num(); ++i)
	{
		UCMItemInstance* Item = InventoryItems[i];

		if (Item && Item->ItemData)
		{
			FString ItemName = Item->ItemData->GetName();
			int32 CurrentQty = Item->Quantity;
			int32 MaxQty = Item->ItemData->MaxStackSize;
			
			FString StatusMsg = FString::Printf(TEXT("[%d] %s (%d/%d)"), 
				i, *ItemName, CurrentQty, MaxQty);

			FColor LogColor = (CurrentQty >= MaxQty) ? FColor::Red : FColor::Green;

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, LogColor, StatusMsg);
				UE_LOG(LogTemp, Log, TEXT("   %s"), *StatusMsg);
			}
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Invalid Slot"));
		}
	}
}