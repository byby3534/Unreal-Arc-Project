#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CMBaseButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCMBaseButtonClicked);

class UButton;
class UTextBlock;
class USoundBase;

UCLASS()
class CRIMSONMOON_API UCMBaseButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Base Button")
	FOnCMBaseButtonClicked OnClicked;

	UFUNCTION(BlueprintCallable, Category = "Base Button|Text")
	void SetButtonText(const FText& InText);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> BaseButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Button|Sound")
	TObjectPtr<USoundBase> ClickSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Button|Sound")
	TObjectPtr<USoundBase> HoverSound;

	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void OnButtonClicked();

	UFUNCTION()
	void OnButtonHovered();
};
