#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "CMCharacterSelectSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSlotSelected, FGameplayTag, CharacterTag);

class UCMBaseButton;
class UTextBlock;
class UImage;
class UBorder;
struct FCharacterSelectInfo;
struct FStreamableHandle;

UCLASS()
class CRIMSONMOON_API UCMCharacterSelectSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSlotCharacterData(const FCharacterSelectInfo& Data);

	void SetIsSelected(bool bIsSelected);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCharacterSlotSelected OnSlotSelected;

	FGameplayTag GetCharacterTag() const { return CharacterTag; }

protected:
	virtual void NativeOnInitialized() override;

	void OnCharacterImageLoaded();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> CharacterSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CharacterName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CharacterImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SelectionBorder;

private:
	FGameplayTag CharacterTag;

	TSharedPtr<FStreamableHandle> ImageLoadHandle;
	TSoftObjectPtr<UTexture2D> SlotImagePtr;

	UFUNCTION()
	void OnCharacterSlotClicked();
};
