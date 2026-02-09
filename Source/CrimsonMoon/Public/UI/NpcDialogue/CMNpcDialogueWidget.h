// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "UI/NpcDialogue/CMNpcDialogueChoiceElement.h"
#include "Npc/CMNpcBase.h" // ECMNpcComponentType 선언 포함
#include "UI/Core/CMBaseWidget.h"
#include "CMNpcDialogueWidget.generated.h"

/**
 * NPC 대화 UI
 */
UCLASS()
class CRIMSONMOON_API UCMNpcDialogueWidget : public UCMBaseWidget
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	UCMNpcDialogueWidget(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* Custom Methods */
public:
	// NPC 이름 텍스트 (디자이너가 Bind할 수 있는 값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta=(ExposeOnSpawn=true))
	FText NpcNameText;

	// 메인 대사 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta=(ExposeOnSpawn=true))
	FText DialogueText;

	// UMG 디자이너에서 위젯과 바인딩할 수 있는 TextBlock 레퍼런스들
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NpcNameTextBlock;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> DialogueTextBlock;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> NextButton;

	// 내부에서 선택 클릭 시 호출될 콜백 (필요시 구현 확장)
	UFUNCTION()
	void HandleChoiceClickedInternal(ECMNpcComponentType ComponentType);
	
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void SetNpcName(const FText& InName) { NpcNameText = InName; if (NpcNameTextBlock) NpcNameTextBlock->SetText(NpcNameText); }

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void SetDialogueText(const FText& InDialogue) { DialogueText = InDialogue; if (DialogueTextBlock) DialogueTextBlock->SetText(DialogueText); }

	// 선택지를 담는 VerticalBox (UMG에서 BindWidget으로 연결)
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ChoicesContainer;

	// 생성된 선택지 위젯들을 보관하는 리스트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<TObjectPtr<UCMNpcDialogueChoiceElement>> ChoiceWidgets;

	// 에디터에서 선택지 위젯 클래스를 설정할 수 있는 프로퍼티
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UCMNpcDialogueChoiceElement> ChoiceElementClass;

	// 선택지 전체를 클리어
	UFUNCTION(BlueprintCallable, Category="Dialogue|Choices")
	void ClearChoices();

	// 단일 선택지 추가 (텍스트와 컴포넌트 타입 전달)
	UFUNCTION(BlueprintCallable, Category="Dialogue|Choices")
	void AddChoice(const FText& InChoiceText, ECMNpcComponentType InComponentType);

protected:
	// Next 버튼 클릭 시 호출되는 함수 (블루프린트에서도 호출 가능)
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void OnNextDialogue();

private:
	UPROPERTY()
	TObjectPtr<UCMLocalEventManager> LocalEventManager = nullptr;
};
