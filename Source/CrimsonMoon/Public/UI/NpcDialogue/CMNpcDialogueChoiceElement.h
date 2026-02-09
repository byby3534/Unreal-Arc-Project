// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/Core/CMBaseWidget.h"
#include "CMNpcDialogueChoiceElement.generated.h"

class UCMLocalEventManager;
class UCMNpcDialogueWidget;
enum class ECMNpcComponentType : uint8;
/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMNpcDialogueChoiceElement : public UUserWidget
{
	GENERATED_BODY()

	/* Engine Methods */
protected:
	virtual void NativeConstruct() override;

	/* Custom Methods */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChoiceClicked);
	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FOnChoiceClicked OnChoiceClicked;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ChoiceButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ChoiceTextBlock;
	
	// 텍스트 세팅용 헬퍼
	FORCEINLINE void SetChoiceText(const FText& InText) { ChoiceText = InText; if (ChoiceTextBlock) ChoiceTextBlock->SetText(ChoiceText); }
	FORCEINLINE void SetChoiceType(ECMNpcComponentType InType) { ChoiceType = InType; }
	FORCEINLINE void SetParentDialogueWidget(UCMNpcDialogueWidget* InWidget) { ParentDialogueWidget = InWidget; }
	

protected:
	// 선택지에 표시될 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText ChoiceText;
	
	UFUNCTION()
	void HandleChoiceButtonClicked();

private:
	UPROPERTY()
	TObjectPtr<UCMNpcDialogueWidget> ParentDialogueWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UCMLocalEventManager> LocalEventManager = nullptr;
	
	ECMNpcComponentType ChoiceType;
};
