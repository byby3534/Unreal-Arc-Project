#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "GameplayTagContainer.h"
#include "CMCharacterSelectWidget.generated.h"

class UCMBaseButton;
class UCMCharacterSelectSlot;
class UCMDataAsset_CharacterSelect;

UCLASS()
class CRIMSONMOON_API UCMCharacterSelectWidget : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	UCMCharacterSelectWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> SelectButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> CancelButton;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UCMCharacterSelectSlot> SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UCMDataAsset_CharacterSelect> CharacterDataAsset;

private:
	UPROPERTY()
	TArray<TObjectPtr<UCMCharacterSelectSlot>> CharacterSlots;

	FGameplayTag CurrentSelectedTag;

	void CreateCharacterSlots();

	UFUNCTION()
	void OnCharacterSlotClicked(FGameplayTag SelectedTag);

	UFUNCTION()
	void OnSelectClicked();

	UFUNCTION()
	void OnCancelClicked();
};
