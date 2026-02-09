#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "Structs/CMStructTypes.h"
#include "CMInteractionWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class CRIMSONMOON_API UCMInteractionWidget : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	UCMInteractionWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateInteractionData(const FInteractionUIData& Data);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateProgressBar(float CurrentProgress);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ActionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TargetText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> InteractionProgressBar;
};
