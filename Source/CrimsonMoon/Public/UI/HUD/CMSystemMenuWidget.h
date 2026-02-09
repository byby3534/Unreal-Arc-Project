#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "CMSystemMenuWidget.generated.h"

class UCMBaseButton;
class UCMSettingWidget;
class UCMBasePopupWidget;

UCLASS()
class CRIMSONMOON_API UCMSystemMenuWidget : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> SettingButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> ExitButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCMSettingWidget> SettingWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCMBasePopupWidget> BasePopupWidgetClass;

private:
	UFUNCTION()
	void OnSettingButtonClicked();

	UFUNCTION()
	void OnMainMenuButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UFUNCTION()
	void ExecuteQuitGame();
};
