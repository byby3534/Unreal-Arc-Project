// Fill out your copyright notice in the Description page of Project Settings.


#include "Npc/Component/CMNpcDialogueComponent.h"

UCMNpcDialogueComponent::UCMNpcDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	NpcComponentType = ECMNpcComponentType::DialogueComponent;
}

void UCMNpcDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
	// 대화 트리는 이제 ACMNpcBase에서 관리
}
