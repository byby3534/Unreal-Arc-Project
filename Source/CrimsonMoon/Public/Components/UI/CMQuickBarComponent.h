// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMQuickBarComponent.generated.h"

struct FGameplayEventData;

class UCMInventoryComponent;
class UCMDataAsset_WeaponData;
class UCMDataAsset_ConsumableData;
class UCMItemInstance;
class UAbilitySystemComponent;

//// 인벤토리 퀵바 UI 갱신 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInvenQuickBarSlotUpdated, int32, GroupIndex, int32, SlotIndex, UCMItemInstance*, ItemInstance);
// UI 갱신 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickBarSlotUpdated, int32, SlotIndex, UCMItemInstance*, ItemInstance);
// 활성 슬롯 변경 알림 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveSlotChanged, int32, NewIndex);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CRIMSONMOON_API UCMQuickBarComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCMQuickBarComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	virtual void BeginPlay() override;

	// 태그 이벤트를 수신할 콜백 함수
	void OnCycleEventReceived(const FGameplayEventData* Payload);

	// 인벤토리가 대체 아이템을 찾았다고 신호를 보내면 실행될 함수
	UFUNCTION()
	void OnRefillItemFound(const UCMDataAsset_ItemBase* OriginalData, UCMItemInstance* NewItem);
	
public:	
	// 아이템 등록 (타입에 따라 슬롯 자동 분류 또는 인덱스 검사)
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	void AssignItemToSlot(int32 SlotIndex, UCMItemInstance* ItemInstance);

	// 특정 슬롯에 아이템 지정 (UI 드래그앤드롭용)
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	void SetQuickBarItem(int32 GroupIndex, int32 SubIndex, UCMItemInstance* ItemInstance);

	UFUNCTION(Server, Reliable)
	void Server_SetQuickBarItem(int32 GroupIndex, int32 SubIndex, UCMItemInstance* ItemInstance);

	// 특정 슬롯 비우기
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	void ClearSlot(int32 GroupIndex, int32 SubIndex);

	UFUNCTION(Server, Reliable)
	void Server_ClearSlot(int32 GroupIndex, int32 SubIndex);

	// 해당 슬롯 활성화
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	void SetActiveSlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_SetActiveSlot(int32 SlotIndex);

	// 현재 활성화된 아이템 인스턴스 반환
	UFUNCTION(BlueprintPure, Category = "QuickBar")
	UCMItemInstance* GetActiveSlotItem() const;

	// 현재 장착된 무기 아이템 반환
	UFUNCTION(BlueprintPure, Category = "QuickBar")
	UCMItemInstance* GetEquippedWeaponItem() const;

	// 해당 슬롯의 아이템을 다음 것으로 교체 (스왑)
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	void CycleSlotItem(int32 SlotIndex, int32 Direction = 1);

	UFUNCTION(Server, Reliable)
	void Server_CycleSlotItem(int32 SlotIndex, int32 Direction);

	// 클라이언트 UI 갱신용 RPC
	UFUNCTION(Client, Reliable)
	void Client_UpdateQuickBarUI(int32 GroupIndex, UCMItemInstance* ItemInstance);

	// 현재 유틸리티 슬롯이 활성화되어 있는지 확인
	UFUNCTION(BlueprintPure, Category = "QuickBar")
	bool IsUtilitySlotActive() const;

	UFUNCTION(BlueprintPure, Category = "QuickBar")
	UCMItemInstance* GetItemInSlot(int32 GroupIndex, int32 SubIndex) const;

	// 인덱스로 아이템 가져오기
	UFUNCTION(BlueprintPure, Category = "QuickBar")
	UCMItemInstance* GetItemAt(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "QuickBar")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	// 현재 활성화된 슬롯 인덱스 반환 (UI 동기화용)
	UFUNCTION(BlueprintPure, Category = "QuickBar")
	int32 GetCurrentActiveSlotIndex() const { return ActiveSlotIndex; }

private:

	// 인벤토리 아이템이 변경되었을 때, 퀵바 슬롯만 콕 집어서 갱신
	UFUNCTION()
	void HandleInventoryItemChanged(UCMItemInstance* ChangedItem);

	// 무기 교체 시 GAS 태그 처리
	void ProcessActiveSlotChange(UCMItemInstance* NewItemInstance);

	// 내부 헬퍼: 빈 서브 슬롯 찾기
	int32 FindEmptySubSlot(const TArray<TObjectPtr<UCMItemInstance>>& SlotArray, int32 MaxCount) const;

	// 아이템이 해당 그룹 슬롯에 장착 가능한지 검사하는 헬퍼 함수
	bool IsItemValidForSlotGroup(int32 GroupIndex, UCMItemInstance* ItemInstance) const;

	// 그룹 인덱스에 따라 해당 슬롯 배열과 활성 서브 인덱스를 반환하는 헬퍼
	TArray<TObjectPtr<UCMItemInstance>>* GetSlotsArray(int32 GroupIndex);
	
	int32 GetActiveSubIndexForGroup(int32 GroupIndex) const;
	
	// 리플리케이션
	UFUNCTION()
	void OnRep_WeaponSlots();
	
	UFUNCTION()
	void OnRep_PotionSlots();
	
	UFUNCTION()
	void OnRep_UtilitySlots();

	UFUNCTION()
	void OnRep_ActiveSlotIndex();
	
	UFUNCTION()
	void OnRep_WeaponActiveIndex();

	UFUNCTION()
	void OnRep_PotionActiveIndex();

	UFUNCTION()
	void OnRep_UtilityActiveIndex();

	UFUNCTION(Client, Reliable)
	void Client_SyncQuickBarSlot(int32 GroupIndex, int32 SubIndex, UCMItemInstance* ItemInstance);

protected:
	// [0번] 무기 슬롯 그룹
	UPROPERTY(ReplicatedUsing = OnRep_WeaponSlots, VisibleAnywhere, BlueprintReadOnly, Category = "QuickBar")
	TArray<TObjectPtr<UCMItemInstance>> WeaponSlots;

	// [0번] 무기 슬롯의 현재 인덱스
	UPROPERTY(ReplicatedUsing = OnRep_WeaponActiveIndex, VisibleAnywhere, BlueprintReadOnly, Category = "QuickBar")
	int32 WeaponActiveIndex = 0;

	// [1번] 포션 슬롯 그룹 (최대 2개 저장)
	UPROPERTY(ReplicatedUsing = OnRep_PotionSlots, VisibleAnywhere, BlueprintReadOnly, Category = "QuickBar")
	TArray<TObjectPtr<UCMItemInstance>> PotionSlots;

	// [1번] 포션 슬롯의 현재 선택 인덱스
	UPROPERTY(ReplicatedUsing = OnRep_PotionActiveIndex, VisibleAnywhere, BlueprintReadOnly, Category = "QuickBar")
	int32 PotionActiveIndex = 0;

	// [2번] 유틸리티 슬롯 그룹 (최대 4개 저장)
	UPROPERTY(ReplicatedUsing = OnRep_UtilitySlots, VisibleAnywhere, BlueprintReadOnly, Category = "QuickBar")
	TArray<TObjectPtr<UCMItemInstance>> UtilitySlots;

	// [2번] 유틸리티 슬롯의 현재 선택 인덱스
	UPROPERTY(ReplicatedUsing = OnRep_UtilityActiveIndex, VisibleAnywhere, BlueprintReadOnly, Category = "QuickBar")
	int32 UtilityActiveIndex = 0;
	
	// 전체 슬롯 수 및 인덱스 정의
	const int32 WeaponSlotIndex = 0;	// 무기
	const int32 PotionSlotIndex = 1;	// 소모품[0]: 포션 (Immediate)
	const int32 UtilitySlotIndex = 2;	// 소모품[1]: 투척/설치 (Throw/Place)

	// 각 슬롯의 최대 용량
	const int32 MaxWeaponCapacity = 2;
	const int32 MaxPotionCapacity = 2;
	const int32 MaxUtilityCapacity = 4;

	// 현재 활성화된 슬롯 인덱스
	UPROPERTY(ReplicatedUsing = OnRep_ActiveSlotIndex, VisibleAnywhere, BlueprintReadOnly, Category = "QuickBar")
	int32 ActiveSlotIndex = -1;

	UPROPERTY()
	TObjectPtr<class UCMInventoryComponent> InventoryComp;
	

public:
	UPROPERTY(BlueprintAssignable, Category = "QuickBar|Events")
	FOnInvenQuickBarSlotUpdated OnInvenQuickBarSlotUpdated;

	UPROPERTY(BlueprintAssignable, Category = "QuickBar|Events")
	FOnQuickBarSlotUpdated OnQuickBarSlotUpdated;

	UPROPERTY(BlueprintAssignable, Category = "QuickBar|Events")
	FOnActiveSlotChanged OnActiveSlotChanged;

	// 디버그 정보를 화면에 출력하는 함수(삭제 예정)
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ShowDebugInfo();
};
