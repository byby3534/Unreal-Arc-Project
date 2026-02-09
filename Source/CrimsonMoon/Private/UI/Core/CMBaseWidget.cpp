#include "UI/Core/CMBaseWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Components/UI/UIManagerComponent.h"
#include "Enums/CMEnums.h"

UCMBaseWidget::UCMBaseWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);

	WidgetConfig.bShowMouseCursor = true;
	WidgetConfig.UILayer = EUILayerType::GameMenu;
	WidgetConfig.UIInputMode = EUIInputMode::GameAndUI;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::SelfHitTestInvisible;
	WidgetConfig.DefaultVisibility = ESlateVisibility::Visible;
	WidgetConfig.bShouldCache = false;
}

void UCMBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayOpenWidget();
}

void UCMBaseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CachedGameInstance = GetGameInstance();
	if (!CachedGameInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[Owner: %s] Failed to get GameInstance!"), *GetName());
		return;
	}

	CachedPlayerController = GetOwningPlayer();
	if (!CachedPlayerController.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[Owner: %s] Failed to get OwningPlayer!"), *GetName());
		return;
	}

	CachedPlayerState = CachedPlayerController->PlayerState;
	if (!CachedPlayerState.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Owner: %s] PlayerState is not ready yet. verify it later."), *GetName());
	}

	UIManager = CachedPlayerController->GetComponentByClass<UUIManagerComponent>();
	if (!IsValid(UIManager))
	{
		UE_LOG(LogTemp, Error, TEXT("[Owner: %s][PC: %s] Missing UIManagerComponent!"), *GetName(), *GetNameSafe(CachedPlayerController.Get()));
	}
}

FReply UCMBaseWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnBackAction();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}


// Lazy Loading
APlayerState* UCMBaseWidget::GetCachedPlayerState()
{
	if (CachedPlayerState.IsValid())
	{
		return CachedPlayerState.Get();
	}

	if (CachedPlayerController.IsValid())
	{
		CachedPlayerState = CachedPlayerController->PlayerState;

		if (CachedPlayerState.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("PlayerState linked lazily."));
		}
	}

	return CachedPlayerState.Get();
}

void UCMBaseWidget::PlayOpenWidget()
{
	if (IsValid(OpenSound))
	{
		UGameplayStatics::PlaySound2D(this, OpenSound);
	}
}

void UCMBaseWidget::PlayCloseWidget()
{
	if (IsValid(CloseSound))
	{
		UGameplayStatics::PlaySound2D(this, CloseSound);
	}
}

void UCMBaseWidget::OnBackAction()
{
	if (IsValid(UIManager))
	{
		UIManager->PopWidget();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Owner: %s] UIManager is missing. Removing widget directly."), *GetName());
		RemoveFromParent();
	}
}