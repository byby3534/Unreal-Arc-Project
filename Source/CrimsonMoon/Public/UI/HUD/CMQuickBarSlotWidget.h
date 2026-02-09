#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CMQuickBarSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UCMItemInstance;

UCLASS()
class CRIMSONMOON_API UCMQuickBarSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateSlot(UCMItemInstance* InItem);

	void SetSlotActive(bool bIsActive);

	void SetSlotIndex(int32 InGroupIndex, int32 InSubIndex = 0);

protected:
	virtual void NativeConstruct() override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> SelectionFrameImage;

	UPROPERTY()
	TWeakObjectPtr<UCMItemInstance> CachedItem;

private:
	int32 GroupIndex;
	int32 SubIndex;
};
