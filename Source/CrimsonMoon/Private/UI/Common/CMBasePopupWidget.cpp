#include "UI/Common/CMBasePopupWidget.h"
#include "UI/Common/CMBaseButton.h"
#include "Components/TextBlock.h"
#include "Components/UI/UIManagerComponent.h"

UCMBasePopupWidget::UCMBasePopupWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetConfig.UILayer = EUILayerType::Popup;
	WidgetConfig.UIInputMode = EUIInputMode::UIOnly;
	WidgetConfig.bShowMouseCursor = true;
	WidgetConfig.bShouldCache = true;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::SelfHitTestInvisible;
}

void UCMBasePopupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UCMBasePopupWidget::OnConfirmClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCMBasePopupWidget::OnCancelClicked);
	}
}


void UCMBasePopupWidget::InitText(const FText& TitleText, const FText& ContentText, const FText& ConfirmText, const FText& CancelText)
{
	if (Title)
	{
		Title->SetText(TitleText);
	}

	if (Content)
	{
		Content->SetText(ContentText);
	}

	if (ConfirmButton)
	{
		ConfirmButton->SetButtonText(ConfirmText);
	}

	if (CancelButton)
	{
		CancelButton->SetButtonText(CancelText);
	}
}

void UCMBasePopupWidget::OnConfirmClicked()
{
	if (ConfirmClicked.IsBound())
	{
		ConfirmClicked.Broadcast();
	}

	if (IsValid(UIManager))
	{
		UIManager->PopWidget();
	}
}

void UCMBasePopupWidget::OnCancelClicked()
{
	if (CancelClicked.IsBound())
	{
		CancelClicked.Broadcast();
	}

	if (IsValid(UIManager))
	{
		UIManager->PopWidget();
	}
}