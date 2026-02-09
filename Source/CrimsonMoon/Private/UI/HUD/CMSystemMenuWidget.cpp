#include "UI/HUD/CMSystemMenuWidget.h"
#include "UI/Common/CMBaseButton.h"
#include "Game/GameInitialization/CMGameInstance.h" 
#include "Kismet/GameplayStatics.h"
#include "UI/Menus/CMSettingWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/UI/UIManagerComponent.h"
#include "UI/Common/CMBasePopupWidget.h"

void UCMSystemMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WidgetConfig.UIInputMode = EUIInputMode::UIOnly;
	WidgetConfig.UILayer = EUILayerType::GameMenu;
	WidgetConfig.bShowMouseCursor = true;
	WidgetConfig.bShouldCache = true;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::Collapsed;

	if (SettingButton)
	{
		SettingButton->OnClicked.AddDynamic(this, &ThisClass::OnSettingButtonClicked);
		SettingButton->SetButtonText(NSLOCTEXT("SystemMenu", "Btn_Setting", "설정"));
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &ThisClass::OnMainMenuButtonClicked);
		MainMenuButton->SetButtonText(NSLOCTEXT("SystemMenu", "Btn_MainMenu", "게임 나가기"));
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
		ExitButton->SetButtonText(NSLOCTEXT("SystemMenu", "Btn_Exit", "게임 종료"));
	}
}

void UCMSystemMenuWidget::OnSettingButtonClicked()
{
	if (!SettingWidgetClass || !UIManager)
	{
		return;
	}

	UIManager->PushWidget(SettingWidgetClass);
}

void UCMSystemMenuWidget::OnMainMenuButtonClicked()
{
	if (UCMGameInstance* GameInstance = Cast<UCMGameInstance>(GetGameInstance()))
	{
		GameInstance->LeaveGame();
	}
	else
	{
		UGameplayStatics::OpenLevel(this, FName("MainMenu"));
	}
}

void UCMSystemMenuWidget::OnExitButtonClicked()
{
	if (!UIManager || !BasePopupWidgetClass)
	{
		ExecuteQuitGame();
		return;
	}

	// 팝업 위젯 푸시
	UCMBasePopupWidget* Popup = Cast<UCMBasePopupWidget>(UIManager->PushWidget(BasePopupWidgetClass));

	if (Popup)
	{
		Popup->InitText(
			NSLOCTEXT("SystemMenu", "ExitGameTitle", "게임 종료"),
			NSLOCTEXT("SystemMenu", "ExitGameDesc", "정말 종료하시겠습니까?"),
			NSLOCTEXT("Common", "Yes", "네"),
			NSLOCTEXT("Common", "No", "아니오"));

		Popup->ConfirmClicked.AddDynamic(this, &ThisClass::ExecuteQuitGame);
	}
}

void UCMSystemMenuWidget::ExecuteQuitGame()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}