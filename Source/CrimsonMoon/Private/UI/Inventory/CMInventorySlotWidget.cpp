#include "UI/Inventory/CMInventorySlotWidget.h"
#include "UI/DragDrop/CMInventoryDragDropOperation.h"
#include "Items/Object/CMItemInstance.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/DragDrop/CMDragItemVisual.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UCMInventorySlotWidget::InitSlot(UCMItemInstance* InItemInstance, int32 InSlotIndex)
{
	ItemInstance = InItemInstance;
	SlotIndex = InSlotIndex;

	if (IconLoadHandle.IsValid())
	{
		IconLoadHandle->CancelHandle();
		IconLoadHandle.Reset();
	}

	if (ItemInstance && ItemInstance->ItemData)
	{
		TSoftObjectPtr<UTexture2D> SoftIcon = ItemInstance->ItemData->ItemIcon;
		if (SoftIcon.IsNull())
		{
			if (IconImage)
			{
				IconImage->SetBrushFromTexture(nullptr);
				IconImage->SetColorAndOpacity(FLinearColor::Transparent);
			}
		}
		else if (SoftIcon.IsValid())
		{
			if (IconImage)
			{
				IconImage->SetBrushFromTexture(SoftIcon.Get());
				IconImage->SetColorAndOpacity(FLinearColor::White);
			}
		}
		else
		{
			if (IconImage)
			{
				IconImage->SetColorAndOpacity(FLinearColor::Transparent);
			}

			UAssetManager& AssetManager = UAssetManager::Get();
			FStreamableManager& Streamable = AssetManager.GetStreamableManager();

			// 비동기 로딩 요청 및 핸들 저장
			IconLoadHandle = Streamable.RequestAsyncLoad(
				SoftIcon.ToSoftObjectPath(),
				FStreamableDelegate::CreateUObject(this, &UCMInventorySlotWidget::OnIconLoaded));
		}

		if (QuantityText)
		{
			if (ItemInstance->Quantity > 1)
			{
				QuantityText->SetText(FText::AsNumber(ItemInstance->Quantity));
				QuantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			else
			{
				QuantityText->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
	else
	{
		if (IconImage)
		{
			IconImage->SetColorAndOpacity(FLinearColor::Transparent);
		}

		if (QuantityText)
		{
			QuantityText->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (HoverFrameImage)
		{
			HoverFrameImage->SetVisibility(ESlateVisibility::HitTestInvisible);
			HoverFrameImage->SetRenderOpacity(0.0f);
		}
	}
}

FReply UCMInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ItemInstance != nullptr)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UCMInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UCMInventoryDragDropOperation* DragOp = NewObject<UCMInventoryDragDropOperation>();
	DragOp->DraggedItemInstance = ItemInstance;
	DragOp->SourceIndex = SlotIndex;

	if (DragVisualClass)
	{
		UCMDragItemVisual* DragItem = CreateWidget<UCMDragItemVisual>(GetWorld(), DragVisualClass);
		if (DragItem)
		{
			DragItem->InitVisual(ItemInstance);
		}

		DragOp->DefaultDragVisual = DragItem;
		DragOp->Pivot = EDragPivot::CenterCenter;
	}

	OutOperation = DragOp;
}

bool UCMInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UCMInventoryDragDropOperation* InventoryOp = Cast<UCMInventoryDragDropOperation>(InOperation);

	if (InventoryOp)
	{
		if (InventoryOp->SourceIndex == SlotIndex)
		{
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("아이템 이동 요청: %d번 슬롯 -> %d번 슬롯"), InventoryOp->SourceIndex, SlotIndex);

		// 예시 코드 (실제 구현 시 주석 해제 및 수정):
		// PC->GetInventoryComponent()->RequestMoveItem(InventoryOp->SourceIndex, SlotIndex);

		return true;
	}

	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UCMInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (HoverFrameImage)
	{
		HoverFrameImage->SetRenderOpacity(0.7f);
	}

	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound);
	}
}

void UCMInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (HoverFrameImage)
	{
		HoverFrameImage->SetRenderOpacity(0.0f);
	}
}

void UCMInventorySlotWidget::OnIconLoaded()
{
	IconLoadHandle.Reset();

	if (ItemInstance && ItemInstance->ItemData && IconImage)
	{
		TSoftObjectPtr<UTexture2D> SoftIcon = ItemInstance->ItemData->ItemIcon;
		if (UTexture2D* TextureAsset = SoftIcon.Get())
		{
			IconImage->SetBrushFromTexture(TextureAsset);
			IconImage->SetColorAndOpacity(FLinearColor::White);
		}
	}
}
