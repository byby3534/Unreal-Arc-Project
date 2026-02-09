#include "UI/HUD/CMQuickBarSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/Object/CMItemInstance.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "UI/DragDrop/CMInventoryDragDropOperation.h"
#include "Components/UI/CMQuickBarComponent.h"

void UCMQuickBarSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GroupIndex = 0;
	SubIndex = 0;

	if (IconImage)
	{
		IconImage->SetVisibility(ESlateVisibility::Hidden);
	}

	if (QuantityText)
	{
		QuantityText->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (SelectionFrameImage)
	{
		SelectionFrameImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool UCMQuickBarSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UCMInventoryDragDropOperation* InventoryOp = Cast<UCMInventoryDragDropOperation>(InOperation);

	if (InventoryOp && InventoryOp->DraggedItemInstance)
	{
		if (APawn* OwningPawn = GetOwningPlayerPawn())
		{
			if (UCMQuickBarComponent* QuickBarComp = OwningPawn->FindComponentByClass<UCMQuickBarComponent>())
			{
				QuickBarComp->Server_SetQuickBarItem(GroupIndex, SubIndex, InventoryOp->DraggedItemInstance);

				return true;
			}
		}
	}

	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

FReply UCMQuickBarSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 우클릭인지 확인
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// 아이템이 있는 경우에만 처리 (선택 사항: 빈 슬롯이어도 서버에 확실히 보내려면 체크 제거 가능)
		if (CachedItem.IsValid())
		{
			if (APawn* OwningPawn = GetOwningPlayerPawn())
			{
				if (UCMQuickBarComponent* QuickBarComp = OwningPawn->FindComponentByClass<UCMQuickBarComponent>())
				{
					// 서버에 해당 슬롯 비우기 요청
					QuickBarComp->Server_ClearSlot(GroupIndex, SubIndex);

					// 입력을 처리했음을 알림 (다른 UI로 이벤트 전파 방지)
					return FReply::Handled();
				}
			}
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UCMQuickBarSlotWidget::UpdateSlot(UCMItemInstance* InItem)
{
	CachedItem = InItem;

	if (InItem && InItem->ItemData)
	{
		if (IconImage)
		{
			UTexture2D* IconTexture = InItem->ItemData->ItemIcon.LoadSynchronous();
			if (IconTexture)
			{
				IconImage->SetBrushFromTexture(IconTexture);
				IconImage->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				IconImage->SetVisibility(ESlateVisibility::Hidden);
			}
		}

		if (QuantityText)
		{
			if (InItem->Quantity > 1)
			{
				QuantityText->SetText(FText::AsNumber(InItem->Quantity));
				QuantityText->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				QuantityText->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	else
	{
		if (IconImage)
		{
			IconImage->SetVisibility(ESlateVisibility::Hidden);
		}

		if (QuantityText)
		{
			QuantityText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UCMQuickBarSlotWidget::SetSlotActive(bool bIsActive)
{
	if (SelectionFrameImage)
	{
		SelectionFrameImage->SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (IconImage)
	{
		IconImage->SetOpacity(bIsActive ? 1.0f : 0.5f);
	}
}

void UCMQuickBarSlotWidget::SetSlotIndex(int32 InGroupIndex, int32 InSubIndex)
{
	GroupIndex = InGroupIndex;
	SubIndex = InSubIndex;
}
