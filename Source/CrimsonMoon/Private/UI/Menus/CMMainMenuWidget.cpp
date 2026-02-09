#include "UI/Menus/CMMainMenuWidget.h"
#include "Components/CanvasPanel.h"
#include "UI/Common/CMBaseButton.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/UI/UIManagerComponent.h"
#include "Game/GameInitialization/CMGameInstance.h"
#include "CrimsonMoon/DebugHelper.h"
#include "UI/Common/CMBasePopupWidget.h"
#include "UI/Menus/CMCharacterSelectWidget.h"
#include "UI/Menus/CMSettingWidget.h"

#define LOCTEXT_NAMESPACE "MainMenuWidget"

UCMMainMenuWidget::UCMMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetConfig.UILayer = EUILayerType::MainMenu;
	WidgetConfig.UIInputMode = EUIInputMode::UIOnly;
}

void UCMMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IsValid(CreateRoomButton))
	{
		CreateRoomButton->OnClicked.AddDynamic(this, &UCMMainMenuWidget::OnCreateRoomClicked);
		CreateRoomButton->SetButtonText(LOCTEXT("CreateRoomButtonText", "게임 생성"));
	}

	if (IsValid(SettingButton))
	{
		SettingButton->OnClicked.AddDynamic(this, &UCMMainMenuWidget::OnSettingButtonClicked);
		SettingButton->SetButtonText(LOCTEXT("SettingButtonText", "설정"));
	}

	if (IsValid(ExitButton))
	{
		ExitButton->OnClicked.AddDynamic(this, &UCMMainMenuWidget::OnExitButtonClicked);
		ExitButton->SetButtonText(LOCTEXT("ExitButtonText", "종료"));
	}
}

void UCMMainMenuWidget::OnBackAction()
{
	OnExitButtonClicked();
}

void UCMMainMenuWidget::OnCreateRoomClicked()
{
	if (!IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] UIManager is invalid."), *GetName());
		return;
	}
	
	if (!IsValid(CharacterSelectWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] BasePopupWidget is invalid."), *GetName());
		return;
	}

	if (UCMGameInstance* CMGI = Cast<UCMGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		CMGI->SetIsHosting(true);
	}
	
	UIManager->PushWidget(CharacterSelectWidget);
}

void UCMMainMenuWidget::OnSettingButtonClicked()
{
	if (!IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] UIManager is invalid."), *GetName());
		return;
	}

	if (!SettingWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SettingWidgetClass is not set! Please check Blueprint."), *GetName());
		return;
	}

	UIManager->PushWidget(SettingWidgetClass);
}

void UCMMainMenuWidget::OnExitButtonClicked()
{
	if (!IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] UIManager is invalid."), *GetName());
		return;
	}

	if (!IsValid(BasePopupWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] BasePopupWidget is invalid."), *GetName());
		return;
	}

	UCMBasePopupWidget* Popup = Cast<UCMBasePopupWidget>(UIManager->PushWidget(BasePopupWidget));

	if (Popup)
	{
		Popup->InitText(
			LOCTEXT("ExitGameTitle", "게임종료"),
			LOCTEXT("ExitGameDesc", "정말 종료하시겠습니까?"),
			LOCTEXT("CommonYes", "네"),
			LOCTEXT("CommonNo", "아니오")
		);
		
		Popup->ConfirmClicked.AddDynamic(this, &UCMMainMenuWidget::ExecuteQuitGame);
	}
}

void UCMMainMenuWidget::ExecuteQuitGame()
{
	if (!GetWorld() || !IsValid(GetPlayerController()))
	{
		return;
	}

	UKismetSystemLibrary::QuitGame(GetWorld(), GetPlayerController(), EQuitPreference::Quit, false);
}

#undef LOCTEXT_NAMESPACE
