// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UI/CMQuickBarComponent.h"
#include "Components/CMInventoryComponent.h"
#include "Items/Object/CMItemInstance.h"
#include "DataAssets/Consumable/CMDataAsset_ConsumableData.h"
#include "DataAssets/Weapon/CMDataAsset_WeaponData.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "CMGameplayTags.h"
#include "Abilities/GameplayAbilityTypes.h"

UCMQuickBarComponent::UCMQuickBarComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCMQuickBarComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(UCMQuickBarComponent, ActiveSlotIndex, COND_OwnerOnly);

	// 무기 배열 및 인덱스
	DOREPLIFETIME_CONDITION(UCMQuickBarComponent, WeaponSlots, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UCMQuickBarComponent, WeaponActiveIndex, COND_OwnerOnly);

	// 포션 배열 및 인덱스
	DOREPLIFETIME_CONDITION(UCMQuickBarComponent, PotionSlots, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UCMQuickBarComponent, PotionActiveIndex, COND_OwnerOnly);

	// 투척형, 설치형 배열 및 인덱스
	DOREPLIFETIME_CONDITION(UCMQuickBarComponent, UtilitySlots, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UCMQuickBarComponent, UtilityActiveIndex, COND_OwnerOnly);
}

bool UCMQuickBarComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// InventoryComponent에서 이미 ItemInstance들을 복제하고 있으므로,
	// QuickBarComponent에서 중복으로 복제하면 클라이언트에서 서로 다른 객체로 인식될 수 있습니다.
	// 따라서 여기서는 복제 코드를 제거하여 InventoryComponent가 관리하는 객체를 참조하도록 합니다.

	return bWroteSomething;
}

void UCMQuickBarComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetOwner()->HasAuthority())
	{
		// 배열 크기 미리 확보 (빈 공간은 nullptr)
		WeaponSlots.Init(nullptr, MaxWeaponCapacity);
		PotionSlots.Init(nullptr, MaxPotionCapacity);
		UtilitySlots.Init(nullptr, MaxUtilityCapacity);
	}

	InventoryComp = GetOwner()->FindComponentByClass<UCMInventoryComponent>();
	if (InventoryComp)
	{
		// 아이템 변경 감지 (서버 및 클라이언트 모두 바인딩)
		InventoryComp->OnInventoryItemChanged.AddDynamic(this, &UCMQuickBarComponent::HandleInventoryItemChanged);
        
		// 리필 아이템 발견 감지 (서버만)
		if (GetOwner()->HasAuthority())
		{
			if (!InventoryComp->OnReplacementItemFound.IsAlreadyBound(this, &UCMQuickBarComponent::OnRefillItemFound))
			{
				InventoryComp->OnReplacementItemFound.AddDynamic(this, &UCMQuickBarComponent::OnRefillItemFound);
			}
		}
	}

	// 오너의 ASC를 가져와서 특정 태그 이벤트가 발생하면 내 함수를 실행하도록 바인딩
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(CMGameplayTags::InputTag_Action_CycleItem)
		   .AddUObject(this, &UCMQuickBarComponent::OnCycleEventReceived);
	}
}

void UCMQuickBarComponent::OnCycleEventReceived(const FGameplayEventData* Payload)
{
	// 클라이언트에서 UI 반응 등을 처리하기 위해 호출됨
	if (Payload)
	{
		int32 Direction = FMath::RoundToInt(Payload->EventMagnitude);
       
		// 현재 활성 슬롯이 무기(0)나 유틸리티(2)일 때만 작동
		if (ActiveSlotIndex == WeaponSlotIndex || ActiveSlotIndex == UtilitySlotIndex)
		{
			CycleSlotItem(ActiveSlotIndex, Direction);
		}
	}
}

void UCMQuickBarComponent::OnRefillItemFound(const UCMDataAsset_ItemBase* OriginalData, UCMItemInstance* NewItem)
{
	if (!NewItem || !OriginalData)
	{
		return;
	}

	// 포션 슬롯 확인
	if (IsItemValidForSlotGroup(PotionSlotIndex, NewItem))
	{
		// 1순위: "다 쓴 아이템(수량 0)"이 들어있는 슬롯을 찾아서 덮어쓰기
		for (int32 i = 0; i < PotionSlots.Num(); ++i)
		{
			UCMItemInstance* SlotItem = PotionSlots[i];
            
			// "아이템이 있고 + 데이터가 같고 + 수량이 0 이하라면" -> 바로 여기가 교체 대상!
			if (SlotItem && SlotItem->ItemData == OriginalData && SlotItem->Quantity <= 0)
			{
				SetQuickBarItem(PotionSlotIndex, i, NewItem);
				UE_LOG(LogTemp, Log, TEXT("[QuickBar] 포션 슬롯 %d번 (소진됨) -> 새 아이템으로 교체 완료"), i);
				return; // 교체했으니 종료
			}
		}

		// 2순위: 다 쓴 게 없다면(이미 비워졌다면), 그냥 빈칸이나 현재 활성 슬롯에 넣음
		// (기존에 작성했던 빈칸 찾기 로직 유지)
		int32 EmptyIndex = FindEmptySubSlot(PotionSlots, MaxPotionCapacity);
		if (EmptyIndex != -1)
		{
			SetQuickBarItem(PotionSlotIndex, EmptyIndex, NewItem);
			return;
		}
	}
	// 유틸리티 슬롯 리필
	else if (IsItemValidForSlotGroup(UtilitySlotIndex, NewItem))
	{
		// 1순위: 다 쓴 아이템 덮어쓰기
		for (int32 i = 0; i < UtilitySlots.Num(); ++i)
		{
			UCMItemInstance* SlotItem = UtilitySlots[i];
            
			if (SlotItem && SlotItem->ItemData == OriginalData && SlotItem->Quantity <= 0)
			{
				SetQuickBarItem(UtilitySlotIndex, i, NewItem);
				UE_LOG(LogTemp, Log, TEXT("[QuickBar] 유틸 슬롯 %d번 (소진됨) -> 새 아이템으로 교체 완료"), i);
				return;
			}
		}

		// 2순위: 빈칸 찾기
		int32 EmptyIndex = FindEmptySubSlot(UtilitySlots, MaxUtilityCapacity);
		if (EmptyIndex != -1)
		{
			SetQuickBarItem(UtilitySlotIndex, EmptyIndex, NewItem);
			return;
		}
	}
}

void UCMQuickBarComponent::CycleSlotItem(int32 SlotIndex, int32 Direction)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_CycleSlotItem(SlotIndex, Direction);
		return;
	}
	
	// [무기 슬롯]
	if (SlotIndex == WeaponSlotIndex)
	{
		WeaponActiveIndex = (WeaponActiveIndex + 1) % MaxWeaponCapacity;
		
		UCMItemInstance* NextWeapon = nullptr;
		if (WeaponSlots.IsValidIndex(WeaponActiveIndex))
		{
			NextWeapon = WeaponSlots[WeaponActiveIndex];
		}
		
		OnQuickBarSlotUpdated.Broadcast(WeaponSlotIndex, NextWeapon);
		
		if (ActiveSlotIndex == WeaponSlotIndex)
		{
			ProcessActiveSlotChange(NextWeapon);
		}
	}
	// [포션 슬롯]
	else if (SlotIndex == PotionSlotIndex)
	{
		PotionActiveIndex = (PotionActiveIndex + 1) % MaxPotionCapacity;
		if (PotionSlots.IsValidIndex(PotionActiveIndex))
		{
			OnQuickBarSlotUpdated.Broadcast(PotionSlotIndex, PotionSlots[PotionActiveIndex]);
		}
	}
	// [유틸리티 슬롯]
	else if (SlotIndex == UtilitySlotIndex)
	{
		UtilityActiveIndex = (UtilityActiveIndex + 1) % MaxUtilityCapacity;
		
		if (UtilitySlots.IsValidIndex(UtilityActiveIndex))
		{
			OnQuickBarSlotUpdated.Broadcast(UtilitySlotIndex, UtilitySlots[UtilityActiveIndex]);
		}
	}
}

void UCMQuickBarComponent::Server_CycleSlotItem_Implementation(int32 SlotIndex, int32 Direction)
{
	CycleSlotItem(SlotIndex, Direction);
}

UCMItemInstance* UCMQuickBarComponent::GetItemAt(int32 SlotIndex) const
{
	if (SlotIndex == WeaponSlotIndex)
	{
		if (WeaponSlots.IsValidIndex(WeaponActiveIndex))
		{
			return WeaponSlots[WeaponActiveIndex];
		}
	}
	else if (SlotIndex == PotionSlotIndex)
	{
		// 포션 그룹의 현재 선택된 아이템 반환
		if (PotionSlots.IsValidIndex(PotionActiveIndex))
		{
			return PotionSlots[PotionActiveIndex];
		}
	}
	else if (SlotIndex == UtilitySlotIndex)
	{
		// 유틸 그룹의 현재 선택된 아이템 반환
		if (UtilitySlots.IsValidIndex(UtilityActiveIndex))
		{
			return UtilitySlots[UtilityActiveIndex];
		}
	}
	return nullptr;
}

void UCMQuickBarComponent::ProcessActiveSlotChange(UCMItemInstance* NewItemInstance)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC)
	{
		return;
	}

	// 현재 무언가 장착 중이라면 '해제 어빌리티' 태그 발동
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(CMGameplayTags::Player_Ability_UnEquipWeapon));
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(CMGameplayTags::Player_Ability_UnEquipConsumable));

	// 아이템이 없으면 종료
	if (!NewItemInstance || !NewItemInstance->ItemData)
	{
		// [디버그 로그 1] 아이템이 없음
		UE_LOG(LogTemp, Warning, TEXT("[QuickBar] 활성 슬롯 변경됨: 아이템 없음 (Empty)"));
		return;
	}

	// 새 아이템이 무기라면 '장착 어빌리티' 태그 발동
	if (UCMDataAsset_WeaponData* WeaponData = Cast<UCMDataAsset_WeaponData>(NewItemInstance->ItemData))
	{
		// 데이터 에셋에 설정된 "이 무기를 장착하는 어빌리티의 태그"를 가져옴
		if (WeaponData->EquipAbilityTriggerTag.IsValid())
		{
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(WeaponData->EquipAbilityTriggerTag));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponData에 EquipAbilityTriggerTag가 설정되지 않았습니다: %s"), *WeaponData->GetName());
		}
	}
	// 소모품 데이터인 경우
	else if (UCMDataAsset_ConsumableData* ConsumableData = Cast<UCMDataAsset_ConsumableData>(NewItemInstance->ItemData))
	{
		// 이벤트 전송 시도
		UE_LOG(LogTemp, Warning, TEXT("[QuickBar] 소모품 장착 이벤트 전송 시도! Tag: %s"), *CMGameplayTags::Player_Ability_EquipConsumable.GetTag().ToString());
		
		static FGameplayTag EquipConsumableTag = CMGameplayTags::Player_Event_EquipConsume;
		
		FGameplayEventData Payload;
		Payload.OptionalObject = NewItemInstance; // ItemInstance를 전달하도록 수정
		Payload.Instigator = GetOwner();
		
		int32 TriggeredCount = ASC->HandleGameplayEvent(EquipConsumableTag, &Payload);
		
		// 전송 결과
		UE_LOG(LogTemp, Warning, TEXT("[QuickBar] 이벤트 반응한 어빌리티 개수: %d"), TriggeredCount);
	}
	
}

void UCMQuickBarComponent::AssignItemToSlot(int32 SlotIndex, UCMItemInstance* ItemInstance)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	// [무기 슬롯]
	if (SlotIndex == WeaponSlotIndex)
	{
		if (ItemInstance == nullptr || (ItemInstance->ItemData && ItemInstance->ItemData->IsA(UCMDataAsset_WeaponData::StaticClass())))
		{
			// 빈 공간 찾기
			int32 TargetIdx = FindEmptySubSlot(WeaponSlots, MaxWeaponCapacity);
			if (TargetIdx == -1)
			{
				TargetIdx = WeaponActiveIndex;
			}

			if (WeaponSlots.Num() <= TargetIdx)
			{
				WeaponSlots.SetNum(MaxWeaponCapacity);
			}
			
			WeaponSlots[TargetIdx] = ItemInstance;

			// 현재 활성 인덱스가 변경되었다면 UI 갱신 및 실제 장착
			if (TargetIdx == WeaponActiveIndex)
			{
				OnQuickBarSlotUpdated.Broadcast(WeaponSlotIndex, ItemInstance);
				
				if (ActiveSlotIndex == WeaponSlotIndex)
				{
					ProcessActiveSlotChange(ItemInstance);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("슬롯 0번에는 무기만 장착할 수 있습니다."));
		}
	}
	// [포션 슬롯]
	else if (SlotIndex == PotionSlotIndex)
	{
		UCMDataAsset_ConsumableData* CData = ItemInstance ? Cast<UCMDataAsset_ConsumableData>(ItemInstance->ItemData) : nullptr;
		
		// 데이터가 없거나(해제), 'Immediate(즉발)' 타입인 경우만 허용
		if (ItemInstance == nullptr || (CData && CData->ConsumableType == EConsumableType::Immediate))
		{

			int32 TargetIdx = FindEmptySubSlot(PotionSlots, MaxPotionCapacity);
			
			if (TargetIdx == -1)
			{
				TargetIdx = PotionActiveIndex;
			}
			
			if (PotionSlots.Num() <= TargetIdx)
			{
				PotionSlots.SetNum(MaxPotionCapacity);
			}
			
			PotionSlots[TargetIdx] = ItemInstance;
			
			// 현재 보고 있는 페이지라면 UI 갱신
			if (TargetIdx == PotionActiveIndex)
			{
				OnQuickBarSlotUpdated.Broadcast(PotionSlotIndex, ItemInstance);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("1번 슬롯에는 즉발형 아이템만 장착 가능합니다."));
		}
	}
	// [유틸리티 슬롯]
	else if (SlotIndex == UtilitySlotIndex)
	{
		UCMDataAsset_ConsumableData* CData = ItemInstance ? Cast<UCMDataAsset_ConsumableData>(ItemInstance->ItemData) : nullptr;

		// 데이터가 없거나(해제), 'Throwable(투척)' 또는 'Placeable(설치)' 타입인 경우만 허용
		if (ItemInstance == nullptr || (CData && (CData->ConsumableType == EConsumableType::Throwable || CData->ConsumableType == EConsumableType::Placeable)))
		{
			int32 TargetIdx = FindEmptySubSlot(UtilitySlots, MaxUtilityCapacity);

			// 빈 공간이 없으면? -> '현재 보고 있는 인덱스'를 덮어씀
			if (TargetIdx == -1)
			{
				TargetIdx = UtilityActiveIndex;
			}

			if (UtilitySlots.Num() <= TargetIdx)
			{
				UtilitySlots.SetNum(MaxUtilityCapacity);
			}

			UtilitySlots[TargetIdx] = ItemInstance;

			if (TargetIdx == UtilityActiveIndex)
			{
				OnQuickBarSlotUpdated.Broadcast(UtilitySlotIndex, ItemInstance);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("2번 슬롯에는 투척/설치형 아이템만 장착 가능합니다."));
		}
	}
}

void UCMQuickBarComponent::SetQuickBarItem(int32 GroupIndex, int32 SubIndex, UCMItemInstance* ItemInstance)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// 유효성 검사
	if (!IsItemValidForSlotGroup(GroupIndex, ItemInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuickBar] Group %d에 적절하지 않은 아이템입니다."), GroupIndex);
		return;
	}

	// 대상 배열 가져오기
	TArray<TObjectPtr<UCMItemInstance>>* TargetSlots = GetSlotsArray(GroupIndex);
	if (!TargetSlots || !TargetSlots->IsValidIndex(SubIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuickBar] 잘못된 슬롯 접근입니다. Group: %d, Sub: %d"), GroupIndex, SubIndex);
		return;
	}

	// 실제 할당
	(*TargetSlots)[SubIndex] = ItemInstance;

	// 상태 갱신
	int32 ActiveSubIndex = GetActiveSubIndexForGroup(GroupIndex);
    
	if (OnInvenQuickBarSlotUpdated.IsBound())
	{
		OnInvenQuickBarSlotUpdated.Broadcast(GroupIndex, SubIndex, ItemInstance);
	}

	if (GetOwner()->GetLocalRole() != ROLE_Authority || GetOwner()->GetRemoteRole() == ROLE_AutonomousProxy)
	{
		Client_SyncQuickBarSlot(GroupIndex, SubIndex, ItemInstance);
	}

	if (SubIndex == ActiveSubIndex)
	{
		OnQuickBarSlotUpdated.Broadcast(GroupIndex, ItemInstance);
	}
    
	if (AActor* Owner = GetOwner())
	{
		Owner->ForceNetUpdate();
		Owner->FlushNetDormancy();
	}

	UE_LOG(LogTemp, Verbose, TEXT("[QuickBar] 슬롯 설정 완료. Group: %d, Sub: %d"), GroupIndex, SubIndex);
}

void UCMQuickBarComponent::Server_SetQuickBarItem_Implementation(int32 GroupIndex, int32 SubIndex, UCMItemInstance* ItemInstance)
{
	SetQuickBarItem(GroupIndex, SubIndex, ItemInstance);
}

void UCMQuickBarComponent::ClearSlot(int32 GroupIndex, int32 SubIndex)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_ClearSlot(GroupIndex, SubIndex);
		return;
	}

	// 대상 배열 가져오기
	TArray<TObjectPtr<UCMItemInstance>>* TargetSlots = GetSlotsArray(GroupIndex);
	if (!TargetSlots || !TargetSlots->IsValidIndex(SubIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuickBar] 잘못된 슬롯 접근입니다. Group: %d, Sub: %d"), GroupIndex, SubIndex);
		return;
	}

	// 슬롯 비우기
	(*TargetSlots)[SubIndex] = nullptr;

	if (OnInvenQuickBarSlotUpdated.IsBound())
	{
		OnInvenQuickBarSlotUpdated.Broadcast(GroupIndex, SubIndex, nullptr);
	}

	if (GetOwner()->GetLocalRole() != ROLE_Authority || GetOwner()->GetRemoteRole() == ROLE_AutonomousProxy)
	{
		Client_SyncQuickBarSlot(GroupIndex, SubIndex, nullptr);
	}

	// 상태 갱신 (UI 및 장착)
	int32 ActiveSubIndex = GetActiveSubIndexForGroup(GroupIndex);

	if (SubIndex == ActiveSubIndex)
	{
		OnQuickBarSlotUpdated.Broadcast(GroupIndex, nullptr);

		// 현재 손에 들고 있는 그룹이라면 실제 메쉬 교체
		if (ActiveSlotIndex == GroupIndex)
		{
			ProcessActiveSlotChange(nullptr);
		}
	}

	if (AActor* Owner = GetOwner())
	{
		Owner->ForceNetUpdate();
		Owner->FlushNetDormancy();
	}

	UE_LOG(LogTemp, Verbose, TEXT("[QuickBar] 슬롯 비우기 완료. Group: %d, Sub: %d"), GroupIndex, SubIndex);
}

void UCMQuickBarComponent::Server_ClearSlot_Implementation(int32 GroupIndex, int32 SubIndex)
{
	ClearSlot(GroupIndex, SubIndex);
}

void UCMQuickBarComponent::SetActiveSlot(int32 SlotIndex)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_SetActiveSlot(SlotIndex);
		return;
	}

	// 유효 범위
	if (SlotIndex != -1 && (SlotIndex < 0 || SlotIndex > UtilitySlotIndex))
	{
		return;
	}
	
	ActiveSlotIndex = SlotIndex;
	
	// 서버인 경우에만 리플리케이션 트리거
	if (GetOwner()->HasAuthority())
	{
		OnRep_ActiveSlotIndex();
	}
	else
	{
		// 클라이언트인 경우 UI 갱신 델리게이트 수동 호출
		OnActiveSlotChanged.Broadcast(ActiveSlotIndex);

		ProcessActiveSlotChange(GetActiveSlotItem());
	}
}

void UCMQuickBarComponent::Server_SetActiveSlot_Implementation(int32 SlotIndex)
{
	SetActiveSlot(SlotIndex);
}

UCMItemInstance* UCMQuickBarComponent::GetActiveSlotItem() const
{
	return GetItemAt(ActiveSlotIndex);
}

UCMItemInstance* UCMQuickBarComponent::GetEquippedWeaponItem() const
{
	if (WeaponSlots.IsValidIndex(WeaponActiveIndex))
	{
		return WeaponSlots[WeaponActiveIndex];
	}
	return nullptr;
}

int32 UCMQuickBarComponent::FindEmptySubSlot(const TArray<TObjectPtr<UCMItemInstance>>& SlotArray, int32 MaxCount) const
{
	for (int32 i = 0; i < MaxCount; ++i)
	{
		if (!SlotArray.IsValidIndex(i) || SlotArray[i] == nullptr)
		{
			return i;
		}
	}
	
	return -1;
}

bool UCMQuickBarComponent::IsItemValidForSlotGroup(int32 GroupIndex, UCMItemInstance* ItemInstance) const
{
	if (ItemInstance == nullptr)
	{
		return true; 
	}

	// 데이터 에셋 캐싱
	const UCMDataAsset_ConsumableData* ConsumableData = Cast<UCMDataAsset_ConsumableData>(ItemInstance->ItemData);

	// [무기 슬롯]
	if (GroupIndex == WeaponSlotIndex)
	{
		return ItemInstance->ItemData && ItemInstance->ItemData->IsA(UCMDataAsset_WeaponData::StaticClass());
	}
	// [포션 슬롯]
	else if (GroupIndex == PotionSlotIndex)
	{
		return ConsumableData && ConsumableData->ConsumableType == EConsumableType::Immediate;
	}
	// [유틸리티 슬롯]
	else if (GroupIndex == UtilitySlotIndex)
	{
		return ConsumableData && (ConsumableData->ConsumableType == EConsumableType::Throwable || 
								  ConsumableData->ConsumableType == EConsumableType::Placeable);
	}

	return false;
}

TArray<TObjectPtr<UCMItemInstance>>* UCMQuickBarComponent::GetSlotsArray(int32 GroupIndex)
{
	if (GroupIndex == WeaponSlotIndex)
	{
		return &WeaponSlots;
	}
	
	if (GroupIndex == PotionSlotIndex)
	{
		return &PotionSlots;
	}
	
	if (GroupIndex == UtilitySlotIndex)
	{
		return &UtilitySlots;
	}
	
	return nullptr;
}

int32 UCMQuickBarComponent::GetActiveSubIndexForGroup(int32 GroupIndex) const
{
	if (GroupIndex == WeaponSlotIndex)
	{
		return WeaponActiveIndex;
	}
	
	if (GroupIndex == PotionSlotIndex)
	{
		return PotionActiveIndex;
	}
	
	if (GroupIndex == UtilitySlotIndex)
	{
		return UtilityActiveIndex;
	}
	
	return -1;
}

void UCMQuickBarComponent::OnRep_WeaponSlots()
{
	if (WeaponSlots.IsValidIndex(WeaponActiveIndex))
	{
		OnQuickBarSlotUpdated.Broadcast(WeaponSlotIndex, WeaponSlots[WeaponActiveIndex]);\
		
		if (ActiveSlotIndex == WeaponSlotIndex)
		{
			ProcessActiveSlotChange(WeaponSlots[WeaponActiveIndex]);
		}
	}
	else
	{
		OnQuickBarSlotUpdated.Broadcast(WeaponSlotIndex, nullptr);
	}
}

void UCMQuickBarComponent::OnRep_PotionSlots()
{
	if (PotionSlots.IsValidIndex(PotionActiveIndex))
	{
		OnQuickBarSlotUpdated.Broadcast(PotionSlotIndex, PotionSlots[PotionActiveIndex]);

		if (ActiveSlotIndex == PotionSlotIndex)
		{
			ProcessActiveSlotChange(PotionSlots[PotionActiveIndex]);
		}
	}
	else
	{
		OnQuickBarSlotUpdated.Broadcast(PotionSlotIndex, nullptr);
	}
}

void UCMQuickBarComponent::OnRep_UtilitySlots()
{
	if (UtilitySlots.IsValidIndex(UtilityActiveIndex))
	{
		OnQuickBarSlotUpdated.Broadcast(UtilitySlotIndex, UtilitySlots[UtilityActiveIndex]);

		if (ActiveSlotIndex == UtilitySlotIndex)
		{
			ProcessActiveSlotChange(UtilitySlots[UtilityActiveIndex]);
		}
	}
	else
	{
		OnQuickBarSlotUpdated.Broadcast(UtilitySlotIndex, nullptr);
	}
}

void UCMQuickBarComponent::HandleInventoryItemChanged(UCMItemInstance* ChangedItem)
{
	if (!ChangedItem)
	{
		return;
	}

	// [서버 전용 로직] 수량이 0이 됨 -> 슬롯 비우기 (자동 리필 제거)
    if (GetOwner()->HasAuthority() && ChangedItem->Quantity <= 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[QuickBar] 아이템 소진됨: %s -> 슬롯 비우기"), *ChangedItem->ItemData->GetName());

        bool bWasEquippedItemRemoved = false;

        // 포션 슬롯 확인
        for (int32 i = 0; i < PotionSlots.Num(); ++i)
        {
            if (PotionSlots[i] == ChangedItem)
            {
                // 빈칸으로 업데이트
                SetQuickBarItem(PotionSlotIndex, i, nullptr);

                // 현재 들고 있던 거였다면 장착 해제 플래그 체크
                if (i == PotionActiveIndex && ActiveSlotIndex == PotionSlotIndex)
                {
                    bWasEquippedItemRemoved = true;
                }
            }
        }
    	
        // 유틸리티 슬롯 확인
        for (int32 i = 0; i < UtilitySlots.Num(); ++i)
        {
            if (UtilitySlots[i] == ChangedItem)
            {
                // 빈칸으로 업데이트
                SetQuickBarItem(UtilitySlotIndex, i, nullptr);

                if (i == UtilityActiveIndex && ActiveSlotIndex == UtilitySlotIndex)
                {
                    bWasEquippedItemRemoved = true;
                }
            }
        }

        // 장착 중이던 아이템이 사라졌으므로 빈손 처리
        if (bWasEquippedItemRemoved)
        {
            ProcessActiveSlotChange(nullptr);
        }

        return;
    }
	else
	{
		// [공통 로직] 수량이 0이 아닐 때 UI 갱신
		// 클라이언트도 OnRep_Quantity -> OnInventoryItemChanged -> 이 함수 호출됨
		for (int32 i = 0; i < PotionSlots.Num(); ++i)
		{
			// 수정: 포인터 비교 대신 ItemData 비교 사용
			if (PotionSlots[i] && PotionSlots[i]->ItemData == ChangedItem->ItemData)
			{
				// 현재 활성 서브 인덱스라면 UI 갱신
				if (i == PotionActiveIndex)
				{
					// 로컬 플레이어(호스트 포함)인 경우 직접 브로드캐스트
					if ((GetOwner()->GetLocalRole() == ROLE_Authority && Cast<APawn>(GetOwner())->IsLocallyControlled()) ||
						GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
					{
						OnQuickBarSlotUpdated.Broadcast(PotionSlotIndex, ChangedItem);
					}
				}
			}
		}

		// 유틸리티 슬롯 확인
		for (int32 i = 0; i < UtilitySlots.Num(); ++i)
		{
			// 수정: 포인터 비교 대신 ItemData 비교 사용
			if (UtilitySlots[i] && UtilitySlots[i]->ItemData == ChangedItem->ItemData)
			{
				if (i == UtilityActiveIndex)
				{
					if ((GetOwner()->GetLocalRole() == ROLE_Authority && Cast<APawn>(GetOwner())->IsLocallyControlled()) ||
						GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
					{
						OnQuickBarSlotUpdated.Broadcast(UtilitySlotIndex, ChangedItem);
					}
				}
			}
		}
	}
}

void UCMQuickBarComponent::Client_UpdateQuickBarUI_Implementation(int32 GroupIndex, UCMItemInstance* ItemInstance)
{
	OnQuickBarSlotUpdated.Broadcast(GroupIndex, ItemInstance);
}

void UCMQuickBarComponent::OnRep_ActiveSlotIndex()
{
	OnActiveSlotChanged.Broadcast(ActiveSlotIndex);

	ProcessActiveSlotChange(GetActiveSlotItem());
}

void UCMQuickBarComponent::OnRep_WeaponActiveIndex()
{
	if (WeaponSlots.IsValidIndex(WeaponActiveIndex))
	{
		// UI 갱신
		OnQuickBarSlotUpdated.Broadcast(WeaponSlotIndex, WeaponSlots[WeaponActiveIndex]);
        
		// 만약 현재 손에 들고 있는 게 무기 슬롯이라면, 3D 모델도 교체
		if (ActiveSlotIndex == WeaponSlotIndex)
		{
			ProcessActiveSlotChange(WeaponSlots[WeaponActiveIndex]);
		}
	}
}

void UCMQuickBarComponent::OnRep_PotionActiveIndex()
{
	if (PotionSlots.IsValidIndex(PotionActiveIndex))
	{
		// UI 갱신
		OnQuickBarSlotUpdated.Broadcast(PotionSlotIndex, PotionSlots[PotionActiveIndex]);
        
		// 포션은 보통 들고 있는 모델이 없거나 ActiveSlotIndex가 일치할 때만 처리
		if (ActiveSlotIndex == PotionSlotIndex)
		{
			ProcessActiveSlotChange(PotionSlots[PotionActiveIndex]);
		}
	}
}

void UCMQuickBarComponent::OnRep_UtilityActiveIndex()
{
	if (UtilitySlots.IsValidIndex(UtilityActiveIndex))
	{
		// UI 갱신
		OnQuickBarSlotUpdated.Broadcast(UtilitySlotIndex, UtilitySlots[UtilityActiveIndex]);

		if (ActiveSlotIndex == UtilitySlotIndex)
		{
			ProcessActiveSlotChange(UtilitySlots[UtilityActiveIndex]);
		}
	}
	else
	{
		OnQuickBarSlotUpdated.Broadcast(UtilitySlotIndex, nullptr);
	}
}

void UCMQuickBarComponent::Client_SyncQuickBarSlot_Implementation(int32 GroupIndex, int32 SubIndex, UCMItemInstance* ItemInstance)
{
	if (OnInvenQuickBarSlotUpdated.IsBound())
	{
		OnInvenQuickBarSlotUpdated.Broadcast(GroupIndex, SubIndex, ItemInstance);
	}
}

bool UCMQuickBarComponent::IsUtilitySlotActive() const
{
	return ActiveSlotIndex == UtilitySlotIndex;
}

UCMItemInstance* UCMQuickBarComponent::GetItemInSlot(int32 GroupIndex, int32 SubIndex) const
{
	const TArray<TObjectPtr<UCMItemInstance>>* TargetSlots = nullptr;

	// 그룹에 따른 배열 선택
	if (GroupIndex == WeaponSlotIndex)
	{
		TargetSlots = &WeaponSlots;
	}
	else if (GroupIndex == PotionSlotIndex)
	{
		TargetSlots = &PotionSlots;
	}
	else if (GroupIndex == UtilitySlotIndex)
	{
		TargetSlots = &UtilitySlots;
	}

	// 유효성 검사 및 반환
	if (TargetSlots && TargetSlots->IsValidIndex(SubIndex))
	{
		return (*TargetSlots)[SubIndex];
	}

	return nullptr;
}

// 삭제 예정
void UCMQuickBarComponent::ShowDebugInfo()
{
	if (!GEngine)
	{
		return;
	}
	
	const float Duration = 5.0f; 
	
	// 무기 슬롯 (Key: 100 ~)
	for (int32 i = 0; i < WeaponSlots.Num(); ++i)
	{
		FString ItemInfo = TEXT("Empty");
		if (WeaponSlots[i] && WeaponSlots[i]->ItemData)
		{
			ItemInfo = FString::Printf(TEXT("%s"), *WeaponSlots[i]->ItemData->ItemName.ToString());
		}

		FString Msg = FString::Printf(TEXT("[Weapon %d] : %s"), i, *ItemInfo);
		FColor Color = FColor::White;

		// 이 무기가 '선택된 무기' 인가?
		if (i == WeaponActiveIndex)
		{
			Msg += TEXT(" ◄");
			Color = (ActiveSlotIndex == WeaponSlotIndex) ? FColor::Green : FColor::Yellow;
		}

		GEngine->AddOnScreenDebugMessage(100 + i, Duration, Color, Msg);
	}

	
	// 포션 슬롯 (Key: 200 ~)
	for (int32 i = 0; i < PotionSlots.Num(); ++i)
	{
		FString ItemInfo = TEXT("Empty");
		if (PotionSlots[i] && PotionSlots[i]->ItemData)
		{
			ItemInfo = FString::Printf(TEXT("%s (x%d)"), *PotionSlots[i]->ItemData->ItemName.ToString(), PotionSlots[i]->Quantity);
		}

		FString Msg = FString::Printf(TEXT("[Potion %d] : %s"), i, *ItemInfo);
		FColor Color = FColor::White;

		if (i == PotionActiveIndex)
		{
			Msg += TEXT(" ◄");
			Color = (ActiveSlotIndex == PotionSlotIndex) ? FColor::Green : FColor::Yellow;
		}

		GEngine->AddOnScreenDebugMessage(200 + i, Duration, Color, Msg);
	}


	// 유틸리티 슬롯 (Key: 300 ~)
	for (int32 i = 0; i < UtilitySlots.Num(); ++i)
	{
		FString ItemInfo = TEXT("Empty");
		if (UtilitySlots[i] && UtilitySlots[i]->ItemData)
		{
			ItemInfo = FString::Printf(TEXT("%s (x%d)"), *UtilitySlots[i]->ItemData->ItemName.ToString(), UtilitySlots[i]->Quantity);
		}

		FString Msg = FString::Printf(TEXT("[Utility %d] : %s"), i, *ItemInfo);
		FColor Color = FColor::White;

		if (i == UtilityActiveIndex)
		{
			Msg += TEXT(" ◄");
			Color = (ActiveSlotIndex == UtilitySlotIndex) ? FColor::Green : FColor::Yellow;
		}

		GEngine->AddOnScreenDebugMessage(300 + i, Duration, Color, Msg);
	}

	// 전체 상태 요약 (Key: 400)
	FString StateMsg = FString::Printf(TEXT("Current Active Main Slot: %d"), ActiveSlotIndex);
	GEngine->AddOnScreenDebugMessage(400, Duration, FColor::Cyan, StateMsg);
}