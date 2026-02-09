// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/CMShopWidget.h"
#include "UI/Shop/CMShopContentElementWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Npc/Component/CMNpcShopComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/CMPlayerController.h"
#include "Components/UI/UIManagerComponent.h"

UCMShopWidget::UCMShopWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetConfig.bShowMouseCursor = true;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::Hidden;
	WidgetConfig.UIInputMode = EUIInputMode::UIOnly;
	WidgetConfig.UILayer = EUILayerType::GameMenu;
}

void UCMShopWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AddItemQuantityButton)
	{
		AddItemQuantityButton->OnClicked.AddDynamic(this, &UCMShopWidget::OnClickedAddItemQuantityButton);
	}

	if (SubtractItemQuantityButton)
	{
		SubtractItemQuantityButton->OnClicked.AddDynamic(this, &UCMShopWidget::OnClickedSubtractItemQuantityButton);
	}

	if (BuyButton)
	{
		BuyButton->OnClicked.AddDynamic(this, &UCMShopWidget::OnClickedBuyButton);
	}

	if (SellButton)
	{
		SellButton->OnClicked.AddDynamic(this, &UCMShopWidget::OnClickedSellButton);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UCMShopWidget::OnClickedCloseButton);
	}

	// 구매 결과 델리게이트 바인딩
	BindToPurchaseDelegate();

	// 초기 리스트 빌드 (디자이너에서 ShopItems를 세팅해 둔 경우)
	BuildShopList();
}

void UCMShopWidget::NativeDestruct()
{
	// 타이머 클리어
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResultMessageTimerHandle);
	}

	UnbindFromPurchaseDelegate();
	Super::NativeDestruct();
}

void UCMShopWidget::SetShopItems(const TArray<FCMShopItemContent>& InItems)
{
	ShopItems = InItems;
	BuildShopList();
}

void UCMShopWidget::HandleElementSelected(UCMShopContentElementWidget* ElementWidget)
{
	// TODO: 아이템이 선택되면 해당 아이템 ID를 저장하거나, 상세 정보 패널을 업데이트하는 등의 작업 수행
	CurrentSelectedItem = ElementWidget->GetItemContent();
	SelectedItemQuantity = 1;
	UpdateSelectedItemDisplay();
}

void UCMShopWidget::UpdateSelectedItemDisplay()
{
	if (SelectedItemNameText)
	{
		SelectedItemNameText->SetText(CurrentSelectedItem.ItemName);
	}

	if (SelectedItemQuantityText)
	{
		SelectedItemQuantityText->SetText(FText::AsNumber(SelectedItemQuantity));
	}

	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(CurrentSelectedItem.ItemDescription);
	}
}

void UCMShopWidget::UpdateSelectedItemQuantityDelta(int32 Delta)
{
	SelectedItemQuantity = FMath::Clamp(SelectedItemQuantity + Delta, 1, 99);
	UpdateSelectedItemDisplay();
}

void UCMShopWidget::OnClickedBuyButton()
{
	if (!CurrentSelectedItem.ItemID.IsNone())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (ACMPlayerController* CMPC = Cast<ACMPlayerController>(PC))
			{
				CMPC->RequestBuyItem(CurrentSelectedItem.ItemID, SelectedItemQuantity);
			}
		}
	}
}

void UCMShopWidget::OnClickedSellButton()
{
	if (!CurrentSelectedItem.ItemID.IsNone())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (ACMPlayerController* CMPC = Cast<ACMPlayerController>(PC))
			{
				CMPC->RequestSellItem(CurrentSelectedItem.ItemID, SelectedItemQuantity);
			}
		}
	}
}

void UCMShopWidget::OnClickedCloseButton()
{
	OnBackAction();
	/*
	if (UIManager)
	{
		UIManager->PopWidget(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UCMShopWidget::OnClickedCloseButton - UIManager is null"));
		RemoveFromParent();
	}
	*/
}

void UCMShopWidget::OnClickedAddItemQuantityButton()
{
	UpdateSelectedItemQuantityDelta(1);
}

void UCMShopWidget::OnClickedSubtractItemQuantityButton()
{
	UpdateSelectedItemQuantityDelta(-1);
}

void UCMShopWidget::BuildShopList()
{
	if (!ShopItemListBox || !ShopItemWidgetClass)
	{
		return;
	}

	ShopItemListBox->ClearChildren();

	for (const FCMShopItemContent& Item : ShopItems)
	{
		UCMShopContentElementWidget* ElementWidget = CreateWidget<UCMShopContentElementWidget>(this, ShopItemWidgetClass);
		if (!ElementWidget)
		{
			continue;
		}

		// 아이템 표시 데이터 세팅 (이미지, 이름, 구매/판매 가격, ID)
		ElementWidget->SetItemDisplayData(Item, this);

		// VerticalBox에 추가하여 리스트로 구성
		if (UVerticalBoxSlot* NewSlot = Cast<UVerticalBoxSlot>(ShopItemListBox->AddChild(ElementWidget)))
		{
			NewSlot->SetHorizontalAlignment(HAlign_Fill);
			NewSlot->SetPadding(FMargin(0.f, 2.f));
		}
	}
}

void UCMShopWidget::BindToPurchaseDelegate()
{
	if (bBoundToPurchaseDelegate)
	{
		return;
	}

	if (ACMPlayerController* CMPC = Cast<ACMPlayerController>(GetPlayerController()))
	{
		CMPC->OnPurchaseResultReceived.AddDynamic(this, &UCMShopWidget::HandlePurchaseResult);
		bBoundToPurchaseDelegate = true;
	}
}

void UCMShopWidget::UnbindFromPurchaseDelegate()
{
	if (!bBoundToPurchaseDelegate)
	{
		return;
	}

	if (ACMPlayerController* CMPC = Cast<ACMPlayerController>(GetPlayerController()))
	{
		CMPC->OnPurchaseResultReceived.RemoveDynamic(this, &UCMShopWidget::HandlePurchaseResult);
	}
	bBoundToPurchaseDelegate = false;
}

void UCMShopWidget::HandlePurchaseResult(const FPurchaseResponse& Response, bool bIsBuy)
{
	// 블루프린트 이벤트 호출
	OnPurchaseResultReceived(Response, bIsBuy);
}

void UCMShopWidget::OnPurchaseResultReceived_Implementation(const FPurchaseResponse& Response, bool bIsBuy)
{
	const FText ResultMessage = GetResultMessageText(Response, bIsBuy);

	// UI에 메시지 표시
	if (ResultMessageText)
	{
		ResultMessageText->SetText(ResultMessage);

		// 성공/실패에 따라 색상 설정
		FSlateColor MessageColor = Response.IsSuccess() ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Red);

		ResultMessageText->SetColorAndOpacity(MessageColor);

		// 기존 타이머 클리어 후 새 타이머 설정
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ResultMessageTimerHandle);
			World->GetTimerManager().SetTimer(
				ResultMessageTimerHandle,
				this,
				&UCMShopWidget::HideResultMessage,
				ResultMessageDisplayDuration,
				false
				);
		}
	}

	// 성공 시 선택 수량 초기화
	if (Response.IsSuccess())
	{
		SelectedItemQuantity = 1;
		UpdateSelectedItemDisplay();
	}
}

void UCMShopWidget::HideResultMessage() const
{
	if (ResultMessageText)
	{
		ResultMessageText->SetText(FText::GetEmpty());
	}
}

FText UCMShopWidget::GetResultMessageText(const FPurchaseResponse& Response, bool bIsBuy) const
{
	const FString ActionType = bIsBuy ? TEXT("구매") : TEXT("판매");

	switch (Response.Result)
	{
		case EPurchaseResult::Success:
			return FText::Format(
				NSLOCTEXT("Shop", "PurchaseSuccess", "{0} 성공: {1} x{2}"),
				FText::FromString(ActionType),
				FText::FromName(Response.ItemID),
				FText::AsNumber(Response.Quantity)
				);

		case EPurchaseResult::InsufficientFunds:
			return NSLOCTEXT("Shop", "InsufficientFunds", "재화가 부족합니다.");

		case EPurchaseResult::InvalidProduct:
			return NSLOCTEXT("Shop", "InvalidProduct", "유효하지 않은 상품입니다.");

		case EPurchaseResult::InvalidQuantity:
			return NSLOCTEXT("Shop", "InvalidQuantity", "잘못된 수량입니다.");

		case EPurchaseResult::InventoryFull:
			return NSLOCTEXT("Shop", "InventoryFull", "인벤토리가 가득 찼습니다.");

		case EPurchaseResult::ItemNotOwned:
			return NSLOCTEXT("Shop", "ItemNotOwned", "보유하지 않은 아이템입니다.");

		case EPurchaseResult::ServerError:
		default:
			return NSLOCTEXT("Shop", "ServerError", "서버 오류가 발생했습니다.");
	}
}