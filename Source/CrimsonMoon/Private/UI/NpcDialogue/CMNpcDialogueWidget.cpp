// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NpcDialogue/CMNpcDialogueWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Game/CMLocalEventManager.h"

UCMNpcDialogueWidget::UCMNpcDialogueWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetConfig.bShowMouseCursor = true;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::Hidden;
	WidgetConfig.UIInputMode = EUIInputMode::UIOnly;
	WidgetConfig.UILayer = EUILayerType::GameMenu;
}

void UCMNpcDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Next 버튼 클릭 바인딩
    if (NextButton)
    {
        NextButton->OnClicked.AddDynamic(this, &UCMNpcDialogueWidget::OnNextDialogue);
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

    if (LocalEventManager)
    {
        // 중복 바인딩 방지: 먼저 제거 후 다시 바인딩
        LocalEventManager->OnCreateChoiceButton.RemoveDynamic(this, &UCMNpcDialogueWidget::AddChoice);
        LocalEventManager->OnCreateChoiceButton.AddDynamic(this, &UCMNpcDialogueWidget::AddChoice);
    }

    // 초기화 시 기존 선택지 정리
    ClearChoices();
}

void UCMNpcDialogueWidget::NativeDestruct()
{
    if (NextButton)
    {
        NextButton->OnClicked.RemoveDynamic(this, &UCMNpcDialogueWidget::OnNextDialogue);
    }

    if (LocalEventManager)
    {
        // 위젯 파괴 시 델리게이트 바인딩 해제
        LocalEventManager->OnCreateChoiceButton.RemoveDynamic(this, &UCMNpcDialogueWidget::AddChoice);
    }

    // 위젯 파괴 시 선택지 정리
    ClearChoices();

    Super::NativeDestruct();
}

void UCMNpcDialogueWidget::HandleChoiceClickedInternal(ECMNpcComponentType ComponentType)
{
	
}

void UCMNpcDialogueWidget::OnNextDialogue()
{
    // TODO: 다음 대화 노드 요청 로직 구현 (현재는 비워 둠)
	UE_LOG(LogTemp, Log, TEXT("OnNextDialogue()"))
	LocalEventManager->OnNextDialogueNodeRequested.Broadcast();
}

void UCMNpcDialogueWidget::ClearChoices()
{
    // VerticalBox 에서 모든 자식 제거
    if (ChoicesContainer)
    {
        ChoicesContainer->ClearChildren();
    }

    // 내부 리스트도 비움
    ChoiceWidgets.Empty();
}

void UCMNpcDialogueWidget::AddChoice(const FText& InChoiceText, ECMNpcComponentType InComponentType)
{
    if (!ChoicesContainer || !GetWorld())
    {
        return;
    }

    // 사용할 위젯 클래스가 설정되어 있는지 확인
    TSubclassOf<UCMNpcDialogueChoiceElement> WidgetClassToUse = ChoiceElementClass;
    if (!WidgetClassToUse)
    {
        // 에디터에서 설정되지 않았다면 기본 클래스로 대체
        WidgetClassToUse = UCMNpcDialogueChoiceElement::StaticClass();
    }

    // 선택지 위젯 생성 (설정된 블루프린트 클래스 사용)
    UCMNpcDialogueChoiceElement* NewChoice = CreateWidget<UCMNpcDialogueChoiceElement>(GetWorld(), WidgetClassToUse);
    if (!NewChoice)
    {
        return;
    }

    // 텍스트 세팅
    NewChoice->SetChoiceText(InChoiceText);
	NewChoice->SetChoiceType(InComponentType);
	NewChoice->SetParentDialogueWidget(this);

    // TODO: 필요 시 InComponentType 을 NewChoice 에 저장하거나 바인딩하는 로직 추가

    // VerticalBox 에 추가
    ChoicesContainer->AddChild(NewChoice);

    // 리스트에 보관
    ChoiceWidgets.Add(NewChoice);
	UE_LOG(LogTemp, Log, TEXT("AddChoice: Added choice with text: %s"), *InChoiceText.ToString());
}
