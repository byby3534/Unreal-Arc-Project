#include "UI/Inventory/CMInventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/CMInventoryComponent.h"
#include "UI/Inventory/CMInventorySlotWidget.h"
#include "UI/Common/CMBaseButton.h"
#include "Controllers/CMPlayerController.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "UI/HUD/CMQuickBarSlotWidget.h"

void UCMInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WidgetConfig.UIInputMode = EUIInputMode::UIOnly;
	WidgetConfig.UILayer = EUILayerType::GameMenu;
	WidgetConfig.bShowMouseCursor = true;
	WidgetConfig.bShouldCache = true;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::Collapsed;

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &ThisClass::OnBackAction);
		CloseButton->SetButtonText(NSLOCTEXT("MyGameUI", "CloseBtn", "닫기"));
	}

	InitQuickBarSlotArrays();
}

void UCMInventoryWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

FReply UCMInventoryWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::I)
	{
		OnBackAction();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UCMInventoryWidget::InitInventoryWidget(UCMInventoryComponent* InInventoryComponent, UCMQuickBarComponent* InQuickBarComponent)
{
	if (IsValid(InInventoryComponent))
	{
		CachedInventoryComp = InInventoryComponent;

		CachedInventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UCMInventoryWidget::UpdateInventory);
		CachedInventoryComp->OnInventoryUpdated.AddDynamic(this, &UCMInventoryWidget::UpdateInventory);

		CachedInventoryComp->OnInventoryItemChanged.RemoveDynamic(this, &UCMInventoryWidget::OnOneItemChanged);
		CachedInventoryComp->OnInventoryItemChanged.AddDynamic(this, &UCMInventoryWidget::OnOneItemChanged);

		UpdateInventory();
	}

	if (IsValid(InQuickBarComponent))
	{
		CachedQuickBarComp = InQuickBarComponent;

		// 델리게이트 바인딩
		CachedQuickBarComp->OnInvenQuickBarSlotUpdated.RemoveDynamic(this, &ThisClass::OnInvenQuickBarSlotUpdated);
		CachedQuickBarComp->OnInvenQuickBarSlotUpdated.AddDynamic(this, &ThisClass::OnInvenQuickBarSlotUpdated);

		// 위젯이 처음 초기화될 때, 현재 퀵바 컴포넌트의 데이터를 가져와 UI에 뿌려줍니다.
		InitQuickBarSlots();
	}
}

void UCMInventoryWidget::UpdateInventory()
{
	if (!ItemSlotGrid || !SlotWidgetClass || !CachedInventoryComp.IsValid())
	{
		return;
	}

	ItemSlotGrid->ClearChildren();

	const TArray<UCMItemInstance*>& Items = CachedInventoryComp->GetInventoryItems();
	int32 MaxSlots = 50;

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		UCMInventorySlotWidget* NewSlot = CreateWidget<UCMInventorySlotWidget>(this, SlotWidgetClass);

		if (NewSlot)
		{
			UCMItemInstance* ItemInst = (Items.IsValidIndex(i)) ? Items[i] : nullptr;

			NewSlot->InitSlot(ItemInst, i);

			int32 Row = i / 5;
			int32 Col = i % 5;
			ItemSlotGrid->AddChildToUniformGrid(NewSlot, Row, Col);
		}
	}
}

void UCMInventoryWidget::InitQuickBarSlots()
{
	if (!CachedQuickBarComp.IsValid())
	{
		return;
	}

	// 예시: 무기 슬롯 갱신
	for (int32 i = 0; i < WeaponUISlots.Num(); ++i)
	{
		if (WeaponUISlots[i])
		{
			WeaponUISlots[i]->UpdateSlot(CachedQuickBarComp->GetItemInSlot(0, i));
		}
	}

	// 예시: 포션 슬롯 갱신
	for (int32 i = 0; i < PotionUISlots.Num(); ++i)
	{
		if (PotionUISlots[i])
		{
			PotionUISlots[i]->UpdateSlot(CachedQuickBarComp->GetItemInSlot(1, i));
		}
	}

	// 예시: 유틸리티 슬롯 갱신
	for (int32 i = 0; i < UtilityUISlots.Num(); ++i)
	{
		if (UtilityUISlots[i])
		{
			UtilityUISlots[i]->UpdateSlot(CachedQuickBarComp->GetItemInSlot(2, i));
		}
	}
}

void UCMInventoryWidget::OnOneItemChanged(UCMItemInstance* ChangedItem)
{
	if (ChangedItem)
	{
		UpdateInventory();

		RefreshQuickBarSlotIfMatches(ChangedItem);
	}
}

void UCMInventoryWidget::OnInvenQuickBarSlotUpdated(int32 GroupIndex, int32 SubIndex, UCMItemInstance* ItemInstance)
{
	TArray<TObjectPtr<UCMQuickBarSlotWidget>>* TargetArray = nullptr;

	// 그룹 인덱스에 따라 사용할 배열 선택
	switch (GroupIndex)
	{
		case 0: // Weapon
			TargetArray = &WeaponUISlots;
			break;
		case 1: // Potion
			TargetArray = &PotionUISlots;
			break;
		case 2: // Utility
			TargetArray = &UtilityUISlots;
			break;
		default:
			return;
	}

	// 선택된 배열이 유효하고, 인덱스가 범위 내에 있으면 업데이트
	if (TargetArray && TargetArray->IsValidIndex(SubIndex))
	{
		if (UCMQuickBarSlotWidget* SlotWidget = (*TargetArray)[SubIndex])
		{
			SlotWidget->UpdateSlot(ItemInstance);
		}
	}
}

void UCMInventoryWidget::InitQuickBarSlotArrays()
{
	// 1. 배열 비우기 (재초기화 대비)
	WeaponUISlots.Empty();
	PotionUISlots.Empty();
	UtilityUISlots.Empty();

	// 2. 무기 슬롯 등록
	WeaponUISlots.Add(WBP_QuickSlot_Weapon_0);
	WeaponUISlots.Add(WBP_QuickSlot_Weapon_1);

	// 3. 포션 슬롯 등록
	PotionUISlots.Add(WBP_QuickSlot_Potion_0);
	PotionUISlots.Add(WBP_QuickSlot_Potion_1);

	// 4. 유틸리티 슬롯 등록
	UtilityUISlots.Add(WBP_QuickSlot_Utility_0);
	UtilityUISlots.Add(WBP_QuickSlot_Utility_1);
	UtilityUISlots.Add(WBP_QuickSlot_Utility_2);
	UtilityUISlots.Add(WBP_QuickSlot_Utility_3);

	// 5. 각 슬롯에 인덱스 정보 주입 (반복문 사용 가능!)
	for (int32 i = 0; i < WeaponUISlots.Num(); ++i)
	{
		if (WeaponUISlots[i])
			WeaponUISlots[i]->SetSlotIndex(0, i);
	}
	for (int32 i = 0; i < PotionUISlots.Num(); ++i)
	{
		if (PotionUISlots[i])
			PotionUISlots[i]->SetSlotIndex(1, i);
	}
	for (int32 i = 0; i < UtilityUISlots.Num(); ++i)
	{
		if (UtilityUISlots[i])
			UtilityUISlots[i]->SetSlotIndex(2, i);
	}
}

void UCMInventoryWidget::RefreshQuickBarSlotIfMatches(UCMItemInstance* ChangedItem)
{
	// 퀵바 컴포넌트가 없으면 검사 불가
	if (!CachedQuickBarComp.IsValid())
	{
		return;
	}

	// 헬퍼 람다 함수: 특정 그룹의 퀵바 슬롯들을 순회하며 아이템이 일치하는지 검사
	// GroupIndex: 0(Weapon), 1(Potion), 2(Utility)
	// TargetUISlots: 업데이트할 UI 위젯 배열
	auto CheckAndRefeshGroup = [&](int32 GroupIndex, const TArray<TObjectPtr<UCMQuickBarSlotWidget>>& TargetUISlots) {
		// UI 슬롯 개수만큼 순회
		for (int32 i = 0; i < TargetUISlots.Num(); ++i)
		{
			// 퀵바 컴포넌트에게 해당 슬롯(Group, Index)에 있는 아이템을 물어봄
			UCMItemInstance* ItemInQuickBar = CachedQuickBarComp->GetItemInSlot(GroupIndex, i);

			// 변경된 아이템(ChangedItem)과 퀵바 슬롯에 있는 아이템(ItemInQuickBar)이 같은 객체인지 포인터 비교
			if (ItemInQuickBar == ChangedItem)
			{
				// 일치한다면 해당 UI 슬롯 갱신!
				if (TargetUISlots.IsValidIndex(i) && TargetUISlots[i])
				{
					TargetUISlots[i]->UpdateSlot(ChangedItem);
				}
			}
		}
	};

	// 각 그룹별로 검사 수행
	CheckAndRefeshGroup(0, WeaponUISlots);
	CheckAndRefeshGroup(1, PotionUISlots);
	CheckAndRefeshGroup(2, UtilityUISlots);
}