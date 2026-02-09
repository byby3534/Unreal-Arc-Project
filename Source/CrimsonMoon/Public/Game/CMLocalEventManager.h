// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CMLocalEventManager.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMLocalEventManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// NPC 대화 델리게이트들을 LocalEventManager 에서 이동
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNextDialogueNodeRequested);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndDialogueNodeRequested);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrintDialogueNodeText, const FText&, DialogueText);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreateChoiceButton, const FText&, DialogueText, ECMNpcComponentType, ComponentType);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedDialogueChoice, ECMNpcComponentType, ComponentType);

	// 다음 대화 노드 요청 브로드캐스트용
	UPROPERTY(BlueprintAssignable, Category="NpcDialogue")
	FOnNextDialogueNodeRequested OnNextDialogueNodeRequested;

	UPROPERTY(BlueprintAssignable, Category="NpcDialogue")
	FOnEndDialogueNodeRequested OnEndDialogueNodeRequested;
	
	// 현재 노드 텍스트 출력 브로드캐스트용
	UPROPERTY(BlueprintAssignable, Category="NpcDialogue")
	FOnPrintDialogueNodeText OnPrintDialogueNodeText;

	UPROPERTY(BlueprintAssignable, Category="NpcDialogue")
	FOnCreateChoiceButton OnCreateChoiceButton;

	UPROPERTY(BlueprintAssignable, Category="NpcDialogue")
	FOnSelectedDialogueChoice OnSelectedDialogueChoice;
};
