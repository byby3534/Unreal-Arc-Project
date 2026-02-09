#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CMDragItemVisual.generated.h"

class UImage;
class UTextBlock;
class UCMItemInstance;

UCLASS()
class CRIMSONMOON_API UCMDragItemVisual : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitVisual(UCMItemInstance* ItemInstance);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuantityText;
};
