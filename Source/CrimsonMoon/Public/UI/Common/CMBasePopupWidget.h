#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "CMBasePopupWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopupButtonDelegate);

class UTextBlock;
class UCMBaseButton;

UCLASS()
class CRIMSONMOON_API UCMBasePopupWidget : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	UCMBasePopupWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Popup")
	void InitText(const FText& Title, const FText& Content, const FText& ConfirmText, const FText& CancelText);

	UPROPERTY(BlueprintAssignable, Category = "Popup|Event")
	FOnPopupButtonDelegate ConfirmClicked;

	UPROPERTY(BlueprintAssignable, Category = "Popup|Event")
	FOnPopupButtonDelegate CancelClicked;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Content;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> CancelButton;

	UFUNCTION()
	void OnConfirmClicked();

	UFUNCTION()
	void OnCancelClicked();
};
