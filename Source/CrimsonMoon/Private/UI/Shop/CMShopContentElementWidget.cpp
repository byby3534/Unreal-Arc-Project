// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/CMShopContentElementWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Npc/CMNpcBase.h"
#include "UI/Shop/CMShopWidget.h"

void UCMShopContentElementWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UCMShopContentElementWidget::HandleContentClicked);
	}
}

void UCMShopContentElementWidget::SetItemDisplayData(const
	FCMShopItemContent& InItem, UCMShopWidget* InShopInstance)
{
	ItemData = InItem;

	if (ItemImage)
	{
		ItemImage->SetBrushFromTexture(ItemData.ItemIcon);
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(ItemData.ItemName);
	}

	if (BuyPriceText)
	{
		BuyPriceText->SetText(FText::AsNumber(ItemData.BuyPrice));
	}

	if (SellPriceText)
	{
		SellPriceText->SetText(FText::AsNumber(ItemData.SellPrice));
	}

	OwningShopInstance = InShopInstance;
}

void UCMShopContentElementWidget::HandleContentClicked()
{
	if (IsValid(OwningShopInstance))
	{
		OwningShopInstance->HandleElementSelected(this);
	}
}