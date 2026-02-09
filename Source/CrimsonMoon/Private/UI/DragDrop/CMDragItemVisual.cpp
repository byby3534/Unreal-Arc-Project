#include "UI/DragDrop/CMDragItemVisual.h"
#include "Items/Object/CMItemInstance.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UCMDragItemVisual::InitVisual(UCMItemInstance* ItemInstance)
{
	if (ItemInstance && ItemInstance->ItemData)
	{
		if (IconImage)
		{
			TSoftObjectPtr<UTexture2D> SoftIcon = ItemInstance->ItemData->ItemIcon;

			UTexture2D* TextureAsset = SoftIcon.LoadSynchronous();

			if (TextureAsset)
			{
				IconImage->SetBrushFromTexture(TextureAsset);
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
	}
}
