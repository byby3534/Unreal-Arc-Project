// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NpcDialogue/CMNpcDialogueChoiceElement.h"

#include "Game/CMLocalEventManager.h"

void UCMNpcDialogueChoiceElement::NativeConstruct()
{
    Super::NativeConstruct();

    if (ChoiceButton)
    {
        ChoiceButton->OnClicked.AddDynamic(this, &UCMNpcDialogueChoiceElement::HandleChoiceButtonClicked);
    }

    // 초기 텍스트 동기화
    if (ChoiceTextBlock)
    {
        ChoiceTextBlock->SetText(ChoiceText);
    }
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	if (!LocalEventManager)
	{
		LocalEventManager = GameInstance->GetSubsystem<UCMLocalEventManager>();
	}
}

void UCMNpcDialogueChoiceElement::HandleChoiceButtonClicked()
{
	if (LocalEventManager)
	{
		LocalEventManager->OnSelectedDialogueChoice.Broadcast(ChoiceType);
	}
}
