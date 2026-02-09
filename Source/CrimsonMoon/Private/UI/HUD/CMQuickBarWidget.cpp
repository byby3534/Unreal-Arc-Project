#include "UI/HUD/CMQuickBarWidget.h"
#include "UI/HUD/CMQuickBarSlotWidget.h"

void UCMQuickBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	QuickBarSlots.Empty();
	QuickBarSlots.Emplace(WBP_WeaponSlot);
	QuickBarSlots.Emplace(WBP_PotionSlot);
	QuickBarSlots.Emplace(WBP_UtilitySlot);

	for (int32 i = 0; i < QuickBarSlots.Num(); ++i)
	{
		if (UCMQuickBarSlotWidget* SlotWidget = QuickBarSlots[i])
		{
			SlotWidget->SetSlotIndex(i);
		}
	}
}

void UCMQuickBarWidget::InitializeQuickBarComponent(UCMQuickBarComponent* InQuickBarComp)
{
	if (!InQuickBarComp)
	{
		return;
	}

	QuickBarComp = InQuickBarComp;

	QuickBarComp->OnQuickBarSlotUpdated.RemoveDynamic(this, &UCMQuickBarWidget::OnQuickBarSlotUpdated);
	QuickBarComp->OnQuickBarSlotUpdated.AddDynamic(this, &UCMQuickBarWidget::OnQuickBarSlotUpdated);

	for (int32 i = 0; i < QuickBarSlots.Num(); ++i)
	{
		if (UCMQuickBarSlotWidget* SlotWidget = QuickBarSlots[i])
		{
			SlotWidget->UpdateSlot(QuickBarComp->GetItemAt(i));
		}
	}
}

void UCMQuickBarWidget::NativeDestruct()
{
	if (QuickBarComp.IsValid())
	{
		QuickBarComp->OnQuickBarSlotUpdated.RemoveDynamic(this, &UCMQuickBarWidget::OnQuickBarSlotUpdated);
	}

	Super::NativeDestruct();
}

void UCMQuickBarWidget::OnQuickBarSlotUpdated(int32 SlotIndex, UCMItemInstance* ItemInstance)
{
	if (QuickBarSlots.IsValidIndex(SlotIndex) && QuickBarSlots[SlotIndex])
	{
		QuickBarSlots[SlotIndex]->UpdateSlot(ItemInstance);
	}
}
