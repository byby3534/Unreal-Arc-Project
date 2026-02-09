#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "CMMainMenuWidget.generated.h"

class UCMBaseButton;
class UCanvasPanel;
class UUIManagerComponent;
class UCMBasePopupWidget;
class UCMCharacterSelectWidget;
class UCMSettingWidget;

UCLASS()
class CRIMSONMOON_API UCMMainMenuWidget : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	UCMMainMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

	virtual void OnBackAction() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> CreateRoomButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> SettingButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> ExitButton;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Popup")
	TSubclassOf<UCMBasePopupWidget> BasePopupWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Widget")
	TSubclassOf<UCMCharacterSelectWidget> CharacterSelectWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Widgets")
	TSubclassOf<UCMSettingWidget> SettingWidgetClass;

private:
	UFUNCTION()
	void OnCreateRoomClicked();

	UFUNCTION()
	void OnSettingButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

#pragma region Popup Actions
private:
	UFUNCTION()
	void ExecuteQuitGame();
#pragma endregion
};
