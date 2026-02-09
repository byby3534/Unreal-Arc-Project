#include "UI/HUD/Interaction/CMInteractionWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

UCMInteractionWidget::UCMInteractionWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetConfig.bShowMouseCursor = false;
	WidgetConfig.UILayer = EUILayerType::HUD;
	WidgetConfig.UIInputMode = EUIInputMode::GameOnly;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::Visible;
}

void UCMInteractionWidget::UpdateInteractionData(const FInteractionUIData& Data)
{
	if (ActionText)
	{
		ActionText->SetText(Data.ActionName);
	}

	if (TargetText)
	{
		TargetText->SetText(Data.TargetName);
	}

	if (KeyText)
	{
		KeyText->SetText(Data.InputKeyText);
	}

	if (InteractionProgressBar)
	{
		if (Data.InteractionType == EInteractionType::Hold)
		{
			InteractionProgressBar->SetVisibility(ESlateVisibility::Visible);
			InteractionProgressBar->SetPercent(Data.CurrentProgress);
		}
		else
		{
			InteractionProgressBar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UCMInteractionWidget::UpdateProgressBar(float CurrentProgress)
{
	if (InteractionProgressBar)
	{
		InteractionProgressBar->SetPercent(CurrentProgress);
	}
}
