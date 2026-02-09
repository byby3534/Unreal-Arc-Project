#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CMInventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UCMItemInstance;
class UCMInventoryDragDropOperation;
struct FStreamableHandle;

UCLASS()
class CRIMSONMOON_API UCMInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitSlot(UCMItemInstance* InItemInstance, int32 InSlotIndex);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void OnIconLoaded();

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UCMItemInstance> ItemInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SlotIndex;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UUserWidget> DragVisualClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> HoverFrameImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Sound")
	TObjectPtr<USoundBase> HoverSound;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuantityText;

	TSharedPtr<FStreamableHandle> IconLoadHandle;
};
