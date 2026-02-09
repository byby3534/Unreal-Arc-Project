#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "CMInventoryWidget.generated.h"

class UUniformGridPanel;
class UCMBaseButton;
class UCMInventoryComponent;
class UCMInventorySlotWidget;
class UCMQuickBarComponent;
class UCMQuickBarSlotWidget;

UCLASS()
class CRIMSONMOON_API UCMInventoryWidget : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	void InitInventoryWidget(UCMInventoryComponent* InInventoryComponent, UCMQuickBarComponent* InQuickBarComponent);

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	UFUNCTION()
	void UpdateInventory();

	void InitQuickBarSlots();

	UFUNCTION()
	void OnOneItemChanged(UCMItemInstance* ChangedItem);

	UFUNCTION()
	void OnInvenQuickBarSlotUpdated(int32 GroupIndex, int32 SubIndex, UCMItemInstance* ItemInstance);

	void InitQuickBarSlotArrays();

	void RefreshQuickBarSlotIfMatches(UCMItemInstance* ChangedItem);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> ItemSlotGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> CloseButton;

	// Group 0: Weapon
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_QuickSlot_Weapon_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_QuickSlot_Weapon_1;

	// Group 1: Potion
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_QuickSlot_Potion_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_QuickSlot_Potion_1;

	// Group 2: Utility
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_QuickSlot_Utility_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_QuickSlot_Utility_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_QuickSlot_Utility_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_QuickSlot_Utility_3;

	TArray<TObjectPtr<UCMQuickBarSlotWidget>> WeaponUISlots;
	TArray<TObjectPtr<UCMQuickBarSlotWidget>> PotionUISlots;
	TArray<TObjectPtr<UCMQuickBarSlotWidget>> UtilityUISlots;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCMInventorySlotWidget> SlotWidgetClass;

private:
	TWeakObjectPtr<UCMInventoryComponent> CachedInventoryComp;
	TWeakObjectPtr<UCMQuickBarComponent> CachedQuickBarComp;
};
