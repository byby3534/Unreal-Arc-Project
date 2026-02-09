#include "UI/Menus/CMCharacterSelectSlot.h"
#include "UI/Common/CMBaseButton.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Engine/Texture2D.h"
#include "DataAssets/UI/CMDataAsset_CharacterSelect.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UCMCharacterSelectSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IsValid(CharacterSlot))
	{
		CharacterSlot->OnClicked.AddDynamic(this, &ThisClass::OnCharacterSlotClicked);
		CharacterSlot->SetButtonText(FText::GetEmpty());
	}

	SetIsSelected(false);
}

void UCMCharacterSelectSlot::OnCharacterImageLoaded()
{
	ImageLoadHandle.Reset();

	if (CharacterImage && SlotImagePtr.IsValid())
	{
		CharacterImage->SetBrushFromTexture(SlotImagePtr.Get());
	}
}

void UCMCharacterSelectSlot::SetSlotCharacterData(const FCharacterSelectInfo& Data)
{
	CharacterTag = Data.CharacterClassTag;
	SlotImagePtr = Data.CharacterImage;

	if (CharacterName)
	{
		CharacterName->SetText(Data.CharacterName);
	}

	if (CharacterSlot)
	{
		TSoftObjectPtr<UTexture2D> SoftImage = Data.CharacterImage;

		if (ImageLoadHandle.IsValid())
		{
			ImageLoadHandle->CancelHandle();
			ImageLoadHandle.Reset();
		}

		if (SoftImage.IsNull())
		{
			CharacterImage->SetBrushFromTexture(nullptr);
		}
		else if (SoftImage.IsValid())
		{
			CharacterImage->SetBrushFromTexture(SoftImage.Get());
		}
		else
		{
			CharacterImage->SetBrushFromTexture(nullptr);

			UAssetManager& AssetManager = UAssetManager::Get();
			FStreamableManager& Streamable = AssetManager.GetStreamableManager();

			ImageLoadHandle = Streamable.RequestAsyncLoad(
				SoftImage.ToSoftObjectPath(),
				FStreamableDelegate::CreateUObject(this, &UCMCharacterSelectSlot::OnCharacterImageLoaded));
		}
	}
}

void UCMCharacterSelectSlot::SetIsSelected(bool bIsSelected)
{
	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(bIsSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);	
	}

	if (CharacterSlot)
	{
		CharacterSlot->SetRenderOpacity(bIsSelected ? 1.0f : 0.5f);
	}
}

void UCMCharacterSelectSlot::OnCharacterSlotClicked()
{
	if (OnSlotSelected.IsBound())
	{
		OnSlotSelected.Broadcast(CharacterTag);
	}
}
