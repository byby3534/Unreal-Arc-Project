#include "Components/UI/UIManagerComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "UI/Core/CMBaseWidget.h"
#include "Components/Input/CMInputComponent.h"
#include "DataAssets/Input/CMDataAsset_InputConfig.h"
#include "CMGameplayTags.h"

UUIManagerComponent::UUIManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bWantsInitializeComponent = true;
}

void UUIManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwningPlayerController = Cast<APlayerController>(GetOwner());
}

void UUIManagerComponent::ShowHUD(TSubclassOf<UUserWidget> HUDClass)
{
	if (!IsValid(OwningPlayerController) || !OwningPlayerController->IsLocalController())
	{
		return;
	}

	if (IsValid(ActiveHUDWidget))
	{
		ActiveHUDWidget->RemoveFromParent();
		ActiveHUDWidget = nullptr;
	}

	ActiveHUDWidget = CreateWidget(OwningPlayerController, HUDClass);
	if (IsValid(ActiveHUDWidget))
	{
		ActiveHUDWidget->AddToViewport(GetZOrderForLayer(EUILayerType::HUD));
	}
}

void UUIManagerComponent::HideHUD()
{
	if (IsValid(ActiveHUDWidget))
	{
		ActiveHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

int32 UUIManagerComponent::GetZOrderForLayer(EUILayerType Layer) const
{
	switch (Layer)
	{
		case EUILayerType::HUD:
			return 0;
		case EUILayerType::GameMenu:
			return 10;
		case EUILayerType::MainMenu:
			return 50;
		case EUILayerType::Popup:
			return 100;
		default:
			return 0;
	}
}

UUserWidget* UUIManagerComponent::PushWidget(TSubclassOf<UUserWidget> WidgetClass, EUILayerType Layer)
{
	if (!IsValid(OwningPlayerController) || !IsValid(WidgetClass) || !OwningPlayerController->IsLocalController())
	{
		return nullptr;
	}

	EUILayerType TargetLayer = Layer;
	ESlateVisibility PreviousVisibility = ESlateVisibility::SelfHitTestInvisible;
	ESlateVisibility TargetVisibility = ESlateVisibility::Visible;
	bool bShouldCache = false;

	if (WidgetClass && WidgetClass->IsChildOf(UCMBaseWidget::StaticClass()))
	{
		const UCMBaseWidget* WidgetCDO = GetDefault<UCMBaseWidget>(WidgetClass);
		PreviousVisibility = WidgetCDO->WidgetConfig.PreviousWidgetVisibility;
		TargetVisibility = WidgetCDO->WidgetConfig.DefaultVisibility;
		bShouldCache = WidgetCDO->WidgetConfig.bShouldCache;

		if (TargetLayer == EUILayerType::None)
		{
			TargetLayer = WidgetCDO->WidgetConfig.UILayer;
		}
	}
	
	// UCMBaseWidget이 아니라면 기본값(GameMenu)적용
	if (TargetLayer == EUILayerType::None)
	{
		TargetLayer = EUILayerType::GameMenu;
	}

	// LayerType이 HUD인 경우 ShowHUD로 넘기고 ActiveHUDWidget 반환
	if (TargetLayer == EUILayerType::HUD)
	{
		ShowHUD(WidgetClass);
		return ActiveHUDWidget;
	}

	if (UUserWidget* PreviousWidget = GetTopWidget())
	{
		PreviousWidget->SetVisibility(PreviousVisibility);
	}

	UUserWidget* NewWidget = nullptr;

	// 캐싱된 위젯인지 확인
	if (bShouldCache)
	{
		if (CachedWidgets.Contains(WidgetClass))
		{
			NewWidget = CachedWidgets[WidgetClass];
			if (!IsValid(NewWidget))
			{
				NewWidget = CreateWidget(OwningPlayerController, WidgetClass);
				CachedWidgets.Add(WidgetClass, NewWidget);
			}
		}
	}
	
	// nullptr인 경우 캐싱된 위젯이 아니기 때문에 새로 생성
	if (!IsValid(NewWidget))
	{
		NewWidget = CreateWidget(OwningPlayerController, WidgetClass);

		if (IsValid(NewWidget) && bShouldCache)
		{
			CachedWidgets.Add(WidgetClass, NewWidget);
		}
	}

	if (!IsValid(NewWidget))
	{
		return nullptr;
	}

	UIStack.Add(NewWidget);

	const int32 BaseZOrder = GetZOrderForLayer(TargetLayer);
	const int32 ResultZOrder = BaseZOrder + UIStack.Num();

	NewWidget->AddToViewport(ResultZOrder);

	NewWidget->SetVisibility(TargetVisibility);

	UpdateInputMode();

	return NewWidget;
}

void UUIManagerComponent::PopWidget(bool bShowPrevious)
{
	if (UIStack.Num() == 0 || !IsValid(OwningPlayerController))
	{
		return;
	}

	if (UUserWidget* PoppedWidget = UIStack.Pop())
	{
		PoppedWidget->RemoveFromParent();
	}

	if (bShowPrevious)
	{
		if (UUserWidget* NewTopWidget = GetTopWidget())
		{
			ESlateVisibility RestoreVisibility = ESlateVisibility::Visible;

			// CMBaseWidget이라면 설정해둔 DefaultVisibility 값을 사용
			if (UCMBaseWidget* CMWidget = Cast<UCMBaseWidget>(NewTopWidget))
			{
				RestoreVisibility = CMWidget->WidgetConfig.DefaultVisibility;
			}

			NewTopWidget->SetVisibility(RestoreVisibility);
		}
	}

	UpdateInputMode();
}

UUserWidget* UUIManagerComponent::GetTopWidget() const
{
	if (UIStack.Num() > 0)
	{
		return UIStack.Top();
	}
	return nullptr;
}

void UUIManagerComponent::OnBackAction()
{
	if (UCMBaseWidget* BaseWidget = Cast<UCMBaseWidget>(GetTopWidget()))
	{
		BaseWidget->OnBackAction();
	}
	else if (GetTopWidget())
	{
		PopWidget();
	}
}

void UUIManagerComponent::ResetUI()
{
	// HUD 정리
	if (IsValid(ActiveHUDWidget))
	{
		ActiveHUDWidget->RemoveFromParent();
		ActiveHUDWidget = nullptr;
	}

	// 스택에 올라간 위젯들 제거
	for (UUserWidget* Widget : UIStack)
	{
		if (IsValid(Widget))
		{
			Widget->RemoveFromParent();
		}
	}
	UIStack.Empty();

	// 캐시된 위젯들은 메모리는 유지하되, Viewport 에서만 제거
	for (auto& Pair : CachedWidgets)
	{
		if (UUserWidget* CachedWidget = Pair.Value)
		{
			CachedWidget->RemoveFromParent();
		}
	}

	// 입력 모드 원복
	UpdateInputMode();
}

void UUIManagerComponent::UpdateInputMode()
{
	if (!IsValid(OwningPlayerController) || !OwningPlayerController->IsLocalController())
	{
		return;
	}

	if (UIStack.Num() > 0)
	{
		if (UUserWidget* TopWidget = GetTopWidget())
		{
			bool bShowMouseCursor = true;
			EUIInputMode CurrentInputMode = EUIInputMode::GameAndUI;
			if (UCMBaseWidget* CMTopWidget = Cast<UCMBaseWidget>(TopWidget))
			{
				CurrentInputMode = CMTopWidget->WidgetConfig.UIInputMode;
				bShowMouseCursor = CMTopWidget->WidgetConfig.bShowMouseCursor;
			}

			switch (CurrentInputMode)
			{
				case EUIInputMode::GameOnly:
				{
					FInputModeGameOnly InputMode;
					OwningPlayerController->SetInputMode(InputMode);
					OwningPlayerController->SetShowMouseCursor(bShowMouseCursor);
				}
				break;
				case EUIInputMode::GameAndUI:
				{
					FInputModeGameAndUI InputMode;
					if (TopWidget)
					{
						InputMode.SetWidgetToFocus(TopWidget->TakeWidget());
					}
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					InputMode.SetHideCursorDuringCapture(false);
					OwningPlayerController->SetInputMode(InputMode);
					OwningPlayerController->SetShowMouseCursor(bShowMouseCursor);
				}
				break;
				case EUIInputMode::UIOnly:
				{
					FInputModeUIOnly InputMode;
					if (TopWidget)
					{
						InputMode.SetWidgetToFocus(TopWidget->TakeWidget());
					}
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					OwningPlayerController->SetInputMode(InputMode);
					OwningPlayerController->SetShowMouseCursor(bShowMouseCursor);
				}
				break;
			}
		}
	}
	else
	{
		FInputModeGameOnly InputMode;
		OwningPlayerController->SetInputMode(InputMode);
		OwningPlayerController->SetShowMouseCursor(false);
	}
}