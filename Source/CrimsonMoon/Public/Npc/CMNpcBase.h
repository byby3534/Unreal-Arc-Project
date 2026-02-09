// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CMNpcHandler.h"
#include "Engine/DataTable.h"
#include "Interfaces/CMInteractableInterface.h"
#include "CMNpcBase.generated.h"

class UCMLocalEventManager;
class UCMNpcComponentBase;
class UCapsuleComponent;
class UCameraComponent;
class UCMNpcDefinition;
class UCMNpcDialogueWidget;
class UCMDataAsset_ItemBase;

UENUM(BlueprintType)
enum class ECMNpcComponentType: uint8
{
	Default UMETA(DisplayName = "Default"),
	DialogueComponent UMETA(DisplayName = "Dialogue Component"),
	QuestComponent UMETA(DisplayName = "Quest Component"),
	ShopComponent UMETA(DisplayName = "Shop Component"),
};

USTRUCT(BlueprintType)
struct FCMShopItemContent: public FTableRowBase
{
	GENERATED_BODY()

	// 아이템을 식별하기 위한 고유 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FName ItemID = NAME_None;

	// 실제 아이템 데이터 에셋 (인벤토리 지급에 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TObjectPtr<UCMDataAsset_ItemBase> ItemDataAsset = nullptr;

	// 구매 가격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 BuyPrice = 0;

	// 판매 가격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 SellPrice = 0;

	// 스택 수량 등으로 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Quantity = 0;

	// 표시용 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText ItemName;

	// 표시용 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TObjectPtr<UTexture2D> ItemIcon = nullptr;

	// 아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText ItemDescription;
};

UCLASS(BlueprintType)
class UCMDialoagueNode : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Dialogue")
	TObjectPtr<UCMDialoagueNode> ParentNode = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<TObjectPtr<UCMDialoagueNode>> Children;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText DialogueText;
};

UCLASS(BlueprintType)
class UCMActionNode : public UCMDialoagueNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	ECMNpcComponentType ActionType;
};

UCLASS()
class CRIMSONMOON_API ACMNpcBase : public AActor, public ICMNpcHandler, public ICMInteractableInterface
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	ACMNpcBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/* Dialogue Methods */
public:

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	UCMDialoagueNode* CreateDialogueNode(
		TSubclassOf<UCMDialoagueNode> NodeClass,
		const FText& InDialogueText
	);

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	UCMDialoagueNode* CreateDialogueNodeWithSettings(
		TSubclassOf<UCMDialoagueNode> NodeClass,
		UCMDialoagueNode* Parent,
		const FText& InDialogueText
	);
	
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	UCMDialoagueNode* CreateActionNodeWithSettings(
		TSubclassOf<UCMActionNode> NodeClass,
		UCMDialoagueNode* Parent,
		const FText& InDialogueText,
		UPARAM(DisplayName="ActionType") ECMNpcComponentType InActionType
	);

	// 현재 노드를 변경하는 함수 (UI에서 사용 예정)
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void SetCurrentNode(UCMDialoagueNode* NewCurrentNode) { CurrentNode = NewCurrentNode; }

	// 자식 인덱스를 이용해 현재 노드를 이동시키는 함수
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool MoveToChildNodeByIndex(int32 ChildIndex);

	UFUNCTION()
	void StartDialogue();

	UFUNCTION()
	void OnNextDialogueNodeRequested();

	UFUNCTION()
	void HandleChoiceSelected(ECMNpcComponentType SelectedActionType);
	
	UFUNCTION()
	void EndDialogue();

	UFUNCTION()
	void PrintCurrentDialogueNodeText();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TObjectPtr<UCMDialoagueNode> RootDialogueNode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Dialogue")
	TArray<TObjectPtr<UCMDialoagueNode>> AllDialogueNodes;
	
	UPROPERTY()
	TObjectPtr<UCMDialoagueNode> CurrentNode = nullptr;

private:
	UPROPERTY()
	TObjectPtr<UCMLocalEventManager> LocalEventManager = nullptr;

	/* Shop Data Methods */
public:
	// 에디터에서 지정할 수 있는 상점 아이템 DataTable (FCMShopItemContent 기반)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<UDataTable> ShopItemDataTable;

	// 이 NPC를 식별하기 위한 ID (NpcWorldSubsystem 등록에 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FName NpcId = NAME_None;

	// NPC 준비 완료 시 상점 데이터를 초기화하는 함수 (예: BeginPlay 이후 호출)
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitializeShopDataFromTable(TArray<FCMShopItemContent>& OutShopItems) const;

	// NpcWorldSubsystem에 자신을 등록/해제하는 헬퍼 함수
	void RegisterToNpcWorldSubsystem();
	void UnregisterFromNpcWorldSubsystem();

	/* Component Methods */
public:
	FORCEINLINE UCameraComponent* GetNpcCameraComponent() const { return NpcCameraComponent; }
	
	// 에디터에서 직접 지정하는 NPC 컴포넌트 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC Settings")
	TArray<TSubclassOf<UCMNpcComponentBase>> NpcComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC Settings")
	TArray<TObjectPtr<UCMNpcComponentBase>> ActiveNpcComponents;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC Settings")
	TObjectPtr<UCameraComponent> NpcCameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC Settings")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC Settings")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;
	
	virtual void PerformInteract(); // 실제 상호작용 처리 로직

	UPROPERTY()
	TMap<ECMNpcComponentType, TObjectPtr<UCMNpcComponentBase>> RegisteredComponentMap;
	
private:
	bool PerformRegisterComponent(ECMNpcComponentType ComponentType, UCMNpcComponentBase* NewComponent);
	
	/* ICMNpcHandler Methods */
public:
	virtual void HandleActionByType(ECMNpcComponentType ComponentType) override;
	virtual bool RegisterComponent(ECMNpcComponentType ComponentType, UCMNpcComponentBase* NewComponent) override;
	
	/* ICMInteractableInterface Methods */
public:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FInteractionUIData GetInteractableData_Implementation() override;
};
