// Fill out your copyright notice in the Description page of Project Settings.


#include "Npc/CMNpcBase.h"
#include "Engine/DataTable.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Npc/Component/CMNpcComponentBase.h"
#include "TimerManager.h"
#include "Game/CMLocalEventManager.h"
#include "Npc/Component/CMNpcShopComponent.h"
#include "Game/GameInitialization/CMNpcWorldSubsystem.h"

ACMNpcBase::ACMNpcBase()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("Root"));

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetupAttachment(RootComponent);
	
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	NpcCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("NpcCameraComponent"));
	NpcCameraComponent->SetupAttachment(RootComponent);
}

void ACMNpcBase::BeginPlay()
{
	Super::BeginPlay();
	
	RegisterToNpcWorldSubsystem();	// 월드 서브시스템에 자신을 등록
	RegisteredComponentMap.Empty();	// 기존 맵 초기화

	// 에디터에서 지정한 NpcComponents 배열을 기반으로 컴포넌트 생성
	ActiveNpcComponents.Empty();	
	for (const TSubclassOf<UCMNpcComponentBase>& ComponentClass : NpcComponents)
	{
		if (*ComponentClass)
		{
			// 이 NPC를 Outer로 해서 컴포넌트 생성
			UCMNpcComponentBase* NewComp = NewObject<UCMNpcComponentBase>(this, ComponentClass);
			if (NewComp)
			{
				// Actor에 등록해서 라이프사이클을 타도록 함
				NewComp->RegisterComponent();

				// 배열에 보관
				ActiveNpcComponents.Add(NewComp);

				// 각 컴포넌트는 BeginPlay에서 스스로 핸들러에 RegisterComponentToHandler를 호출함
			}
		}
	}
	
	if (RegisteredComponentMap.Contains(ECMNpcComponentType::ShopComponent))
	{
		if (UCMNpcShopComponent* ShopComp = Cast<UCMNpcShopComponent>(RegisteredComponentMap[ECMNpcComponentType::ShopComponent]))
		{
			TArray<FCMShopItemContent> OutShopItems;
			InitializeShopDataFromTable(OutShopItems);
			ShopComp->SetShopItemContents(OutShopItems);
		}
	}
}

void ACMNpcBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 월드 서브시스템에서 자신을 해제
	UnregisterFromNpcWorldSubsystem();

	Super::EndPlay(EndPlayReason);
}

void ACMNpcBase::RegisterToNpcWorldSubsystem()
{
	if (NpcId.IsNone())
	{
		// ID가 설정되지 않았으면 등록하지 않음
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UCMNpcWorldSubsystem* NpcSubsystem = World->GetSubsystem<UCMNpcWorldSubsystem>())
	{
		NpcSubsystem->RegisterNpc(NpcId, this);
	}
}

void ACMNpcBase::UnregisterFromNpcWorldSubsystem()
{
	if (NpcId.IsNone())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UCMNpcWorldSubsystem* NpcSubsystem = World->GetSubsystem<UCMNpcWorldSubsystem>())
	{
		NpcSubsystem->UnregisterNpc(NpcId);
	}
}

void ACMNpcBase::PerformInteract()
{
	// NPC와 상호작용 시 대화 UI를 표시
	StartDialogue();
}

UCMDialoagueNode* ACMNpcBase::CreateDialogueNode(TSubclassOf<UCMDialoagueNode> NodeClass, const FText& InDialogueText)
{
	if (!*NodeClass)
	{
		NodeClass = UCMDialoagueNode::StaticClass();
	}

	UCMDialoagueNode* NewNode = NewObject<UCMDialoagueNode>(this, NodeClass);
	if (NewNode)
	{
		// 생성 시점에 텍스트 설정
		NewNode->DialogueText = InDialogueText;

		AllDialogueNodes.Add(NewNode);

		if (!RootDialogueNode)
		{
			RootDialogueNode = NewNode;
			CurrentNode = RootDialogueNode;
		}
	}

	return NewNode;
}

UCMDialoagueNode* ACMNpcBase::CreateDialogueNodeWithSettings(
	TSubclassOf<UCMDialoagueNode> NodeClass,
	UCMDialoagueNode* Parent,
	const FText& InDialogueText)
{
	// 기본 CreateDialogueNode를 통해 생성 + 텍스트 설정까지 한 번에 처리
	UCMDialoagueNode* NewNode = CreateDialogueNode(NodeClass, InDialogueText);
	if (!NewNode)
	{
		return nullptr;
	}

	if (Parent)
	{
		NewNode->ParentNode = Parent;
		Parent->Children.Add(NewNode);
	}

	return NewNode;
}

UCMDialoagueNode* ACMNpcBase::CreateActionNodeWithSettings(
	TSubclassOf<UCMActionNode> NodeClass,
	UCMDialoagueNode* Parent,
	const FText& InDialogueText,
	ECMNpcComponentType InActionType)
{
	if (!*NodeClass)
	{
		NodeClass = UCMActionNode::StaticClass();
	}

	UCMActionNode* NewNode = NewObject<UCMActionNode>(this, NodeClass);

	// 액션 타입 설정
	NewNode->ActionType = InActionType;
	NewNode->DialogueText = InDialogueText;

	if (Parent)
	{
		NewNode->ParentNode = Parent;
		Parent->Children.Add(NewNode);
	}

	return NewNode;
}

static void LogAndProcessDialogueNodeRecursive(ACMNpcBase* OwnerNpc, const UCMDialoagueNode* Node, int32& Index)
{
	if (!Node)
	{
		return;
	}

	// 현재 노드 로그 출력
	const FString TextString = Node->DialogueText.ToString();
	UE_LOG(LogTemp, Log, TEXT("[DialogueNode %d] %s"), Index, *TextString);

	// Action 노드인 경우 HandleActionByType 호출
	if (const UCMActionNode* ActionNode = Cast<UCMActionNode>(Node))
	{
		if (OwnerNpc)
		{
			UE_LOG(LogTemp, Log, TEXT("-> Handling action of type %d"), static_cast<int32>(ActionNode->ActionType));
			OwnerNpc->HandleActionByType(ActionNode->ActionType);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[DialogueNode %d] Not ActionNode!"), Index);
	}

	++Index;

	for (const UCMDialoagueNode* Child : Node->Children)
	{
		LogAndProcessDialogueNodeRecursive(OwnerNpc, Child, Index);
	}
}

bool ACMNpcBase::MoveToChildNodeByIndex(int32 ChildIndex)
{
	if (!CurrentNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToChildNodeByIndex: CurrentNode is null"));
		return false;
	}

	if (!CurrentNode->Children.IsValidIndex(ChildIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToChildNodeByIndex: Invalid child index %d"), ChildIndex);
		return false;
	}

	UCMDialoagueNode* NextNode = CurrentNode->Children[ChildIndex];
	if (!NextNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToChildNodeByIndex: Child at index %d is null"), ChildIndex);
		return false;
	}

	CurrentNode = NextNode;
	return true;
}

void ACMNpcBase::StartDialogue()
{
	// GameInstanceSubsystem에서 LocalEventManager 가져오기
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
		LocalEventManager->OnSelectedDialogueChoice.AddDynamic(this, &ACMNpcBase::HandleChoiceSelected);
		LocalEventManager->OnNextDialogueNodeRequested.AddDynamic(this, &ACMNpcBase::OnNextDialogueNodeRequested);
		LocalEventManager->OnPrintDialogueNodeText.Broadcast(RootDialogueNode->DialogueText);
	}
}

void ACMNpcBase::OnNextDialogueNodeRequested()
{
	if (!LocalEventManager)
	{
		return;
	}

	if (!CurrentNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnNextDialogueNodeRequested: CurrentNode is null"));
		return;
	}

	const int32 NumChildren = CurrentNode->Children.Num();

	if (NumChildren == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("OnNextDialogueNodeRequested: No more child node. End dialogue."));
		EndDialogue();
		return;
	}

	if (NumChildren != 1)
	{
		UE_LOG(LogTemp, Log, TEXT("OnNextDialogueNodeRequested: Child count is %d (expected 1). No move."), NumChildren);
		return;
	}
	
	UCMDialoagueNode* NextNode = CurrentNode->Children[0];
	if (!NextNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnNextDialogueNodeRequested: NextNode is null"));
		return;
	}

	CurrentNode = NextNode;
	LocalEventManager->OnPrintDialogueNodeText.Broadcast(CurrentNode->DialogueText);

	for (UCMDialoagueNode* ChildNode : CurrentNode->Children)
	{
		if (!ChildNode)
		{
			continue;
		}

		if (UCMActionNode* ActionNode = Cast<UCMActionNode>(ChildNode))
		{
			// 액션 노드의 텍스트와 타입을 전달
			LocalEventManager->OnCreateChoiceButton.Broadcast(ActionNode->DialogueText, ActionNode->ActionType);
		}
	}
}

void ACMNpcBase::HandleChoiceSelected(ECMNpcComponentType SelectedActionType)
{
	EndDialogue();
	HandleActionByType(SelectedActionType);
}

void ACMNpcBase::EndDialogue()
{
	if (LocalEventManager)
	{
		LocalEventManager->OnEndDialogueNodeRequested.Broadcast();
		
		LocalEventManager->OnSelectedDialogueChoice.RemoveDynamic(this, &ACMNpcBase::HandleChoiceSelected);
		LocalEventManager->OnNextDialogueNodeRequested.RemoveDynamic(this, &ACMNpcBase::OnNextDialogueNodeRequested);
		LocalEventManager->OnEndDialogueNodeRequested.RemoveDynamic(this, &ACMNpcBase::EndDialogue);
	}

	CurrentNode = RootDialogueNode;
}

void ACMNpcBase::PrintCurrentDialogueNodeText()
{

}

bool ACMNpcBase::PerformRegisterComponent(ECMNpcComponentType ComponentType, UCMNpcComponentBase* NewComponent)
{
	if (RegisteredComponentMap.Contains(ComponentType))
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformRegisterComponent: ComponentType %d is already registered."), static_cast<int32>(ComponentType));
		return false;
	}

	RegisteredComponentMap.Add(ComponentType, NewComponent);
	UE_LOG(LogTemp, Log, TEXT("PerformRegisterComponent: Registered ComponentType %d."), static_cast<int32>(ComponentType));
	return true;
}

void ACMNpcBase::HandleActionByType(ECMNpcComponentType ComponentType)
{
	if (RegisteredComponentMap.Contains(ComponentType))
	{
		if (UCMNpcComponentBase* Component = RegisteredComponentMap[ComponentType])
		{
			Component->PerformAction();
		}
	}
}

bool ACMNpcBase::RegisterComponent(ECMNpcComponentType ComponentType, UCMNpcComponentBase* NewComponent)
{
	return PerformRegisterComponent(ComponentType, NewComponent);
}

void ACMNpcBase::InitializeShopDataFromTable(TArray<FCMShopItemContent>& OutShopItems) const
{
	OutShopItems.Reset();

	if (!ShopItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACMNpcBase::InitializeShopDataFromTable: ShopItemDataTable is not set on %s"), *GetName());
		return;
	}

	// NOTE: FCMShopItemContent 기반 DataTable 이라고 가정하고 전체 Row를 읽어옴
	const TArray<FName> RowNames = ShopItemDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (const FCMShopItemContent* Row = ShopItemDataTable->FindRow<FCMShopItemContent>(RowName, TEXT("InitializeShopDataFromTable")))
		{
			OutShopItems.Add(*Row);
		}
	}
}

void ACMNpcBase::Interact_Implementation(AActor* Interactor)
{
	// 플레이어가 이 NPC와 상호작용했을 때 호출
	PerformInteract();
}

FInteractionUIData ACMNpcBase::GetInteractableData_Implementation()
{
	FInteractionUIData Data;

	Data.ActionName = FText::FromString(TEXT("대화하기"));

	// NOTE: NPC 이름 설정을 위한 방법 필요
	if (!NpcId.IsNone())
	{
		Data.TargetName = FText::FromName(NpcId);
	}
	else
	{
		Data.TargetName = FText::FromString(TEXT("NPC"));
	}

	// 입력 키 텍스트는 UI 쪽에서 교체할 수 있으므로 기본값만 설정
	Data.InputKeyText = FText::FromString(TEXT("2"));

	// 즉시 상호작용 타입으로 설정
	Data.InteractionType = EInteractionType::Instant;
	Data.InteractionDuration = 0.0f;
	Data.CurrentProgress = 0.0f;

	return Data;
}