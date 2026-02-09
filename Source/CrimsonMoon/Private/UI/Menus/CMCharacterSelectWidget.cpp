#include "UI/Menus/CMCharacterSelectWidget.h"
#include "UI/Common/CMBaseButton.h"
#include "UI/Menus/CMCharacterSelectSlot.h"
#include "DataAssets/UI/CMDataAsset_CharacterSelect.h"
#include "Components/PanelWidget.h"
#include "Game/GameInitialization/CMGameInstance.h"

#define LOCTEXT_NAMESPACE "CharacterSelectWidget"

UCMCharacterSelectWidget::UCMCharacterSelectWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetConfig.UIInputMode = EUIInputMode::UIOnly;
	WidgetConfig.UILayer = EUILayerType::MainMenu;
	WidgetConfig.bShowMouseCursor = true;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::Collapsed;
}

void UCMCharacterSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IsValid(SelectButton))
	{
		SelectButton->OnClicked.AddDynamic(this, &ThisClass::OnSelectClicked);
		SelectButton->SetButtonText(LOCTEXT("SelectButtonText", "선택"));
		SelectButton->SetIsEnabled(false);
	}

	if (IsValid(CancelButton))
	{
		CancelButton->OnClicked.AddDynamic(this, &ThisClass::OnCancelClicked);
		CancelButton->SetButtonText(LOCTEXT("CancelButtonText", "취소"));
	}

	CreateCharacterSlots();
}

void UCMCharacterSelectWidget::CreateCharacterSlots()
{
	if (!SlotContainer || !SlotWidgetClass || !CharacterDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterSelectWidget Config Missing"));
		return;
	}

	for (const FCharacterSelectInfo& Info : CharacterDataAsset->CharacterOptions)
	{
		UCMCharacterSelectSlot* NewSlot = CreateWidget<UCMCharacterSelectSlot>(this, SlotWidgetClass);
		if (NewSlot)
		{
			NewSlot->SetSlotCharacterData(Info);
			NewSlot->OnSlotSelected.AddDynamic(this, &ThisClass::OnCharacterSlotClicked);

			SlotContainer->AddChild(NewSlot);
			CharacterSlots.Add(NewSlot);
		}
	}
}

void UCMCharacterSelectWidget::OnCharacterSlotClicked(FGameplayTag SelectedTag)
{
	CurrentSelectedTag = SelectedTag;
	
	UE_LOG(LogTemp, Warning, TEXT("CurrentTag -> %s"), *CurrentSelectedTag.ToString());

	for (UCMCharacterSelectSlot* CharacterSlot : CharacterSlots)
	{
		if (CharacterSlot)
		{
			bool bIsSelected = CharacterSlot->GetCharacterTag() == SelectedTag;
			CharacterSlot->SetIsSelected(bIsSelected);
		}
	}

	if (SelectButton)
	{
		SelectButton->SetIsEnabled(true);
	}
}

void UCMCharacterSelectWidget::OnSelectClicked()
{
	if (UCMGameInstance* GI = Cast<UCMGameInstance>(CachedGameInstance))
	{
		GI->SetSelectedCharacterTag(CurrentSelectedTag);
		GI->TryJoinOrHostSession();
	}
}

void UCMCharacterSelectWidget::OnCancelClicked()
{
	OnBackAction();
}

#undef LOCTEXT_NAMESPACE