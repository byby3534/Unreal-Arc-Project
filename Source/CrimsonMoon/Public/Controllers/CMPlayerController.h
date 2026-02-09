#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Controllers/CMPlayerControllerBase.h"
#include "Enums/CMEnums.h"
#include "GameplayTagContainer.h"
#include "Structs/CMShopTypes.h"
#include "CMPlayerController.generated.h"

struct FCMShopItemContent;
class UCMShopWidget;
class UCMLocalEventManager;
class UCMNpcDialogueWidget;
class ACMNpcBase;

/**
 *
 */
UCLASS()
class CRIMSONMOON_API ACMPlayerController : public ACMPlayerControllerBase, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ACMPlayerController();

	virtual FGenericTeamId GetGenericTeamId() const override;

	void SetTeamId(ETeamType NewTeamType);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void SetupInputComponent() override;

	/* Npc Dialogue UI */
private:
	UPROPERTY(Replicated)
	uint8 TeamId = static_cast<uint8>(ETeamType::Player); // 기본값 설정

	#pragma region NPC Dialogue UI

public:
	// 에디터에서 바인딩할 수 있는 NPC 대화 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Npc|UI")
	TSubclassOf<UCMNpcDialogueWidget> NpcDialogueWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UCMNpcDialogueWidget> NpcDialogueWidgetInstance;

	UFUNCTION(BlueprintCallable, Category="Npc|UI")
	void CreateNpcDialogueWidget();

	UFUNCTION(BlueprintCallable, Category="Npc|UI")
	void RemoveNpcDialogueWidget();

	UFUNCTION()
	void OnNextDialogueNodeRequested(const FText& DialogueText);

private:
	void InitializeNpcDialogueEvent();

	UPROPERTY()
	TObjectPtr<UCMLocalEventManager> LocalEventManager = nullptr;
	#pragma endregion

	#pragma region Shop UI

public:
	// 상점 UI 생성을 요청하는 클라이언트 함수 (NpcId 기반)
	UFUNCTION(BlueprintCallable, Category = "Shop|UI")
	void RequestOpenShopUI(const FName& NpcId);

	UFUNCTION()
	void RequestBuyItem(const FName& ItemId, int32 Quantity);
	UFUNCTION()
	void RequestSellItem(const FName& ItemId, int32 Quantity);

	UFUNCTION(Server, Reliable)
	void Server_RequestBuyItem(const FName& ItemId, int32 Quantity);
	UFUNCTION(Server, Reliable)
	void Server_RequestSellItem(const FName& ItemId, int32 Quantity);

protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestShopData(const FName& NpcId);

	// 서버에서 보낸 Shop 데이터를 받아 UI 를 여는 클라이언트 RPC
	UFUNCTION(Client, Reliable)
	void Client_ReceiveShopDataAndOpen(const TArray<FCMShopItemContent>& ShopItems);

public:
	// 상점 위젯을 생성하고 Viewport 에 추가 (데이터 입력 포함)
	UFUNCTION(BlueprintCallable, Category = "Shop|UI")
	void CreateShopWidget(const TArray<FCMShopItemContent>& InShopItems);

	// 상점 위젯을 Viewport 에서 제거 및 인스턴스 정리
	UFUNCTION(BlueprintCallable, Category = "Shop|UI")
	void RemoveShopWidget();

protected:
	// 에디터에서 선택할 수 있는 상점 메인 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|UI")
	TSubclassOf<UCMShopWidget> ShopWidgetClass;

	// 런타임에 생성되는 상점 위젯 인스턴스
	UPROPERTY(Transient)
	TObjectPtr<UCMShopWidget> ShopWidgetInstance;

	// 서버 -> 클라이언트: 구매/판매 결과 전송
	UFUNCTION(Client, Reliable)
	void Client_ReceivePurchaseResult(const FPurchaseResponse& Response, bool bIsBuy);

public:
	// 구매/판매 결과 델리게이트 (UI 바인딩용)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPurchaseResultReceived, const FPurchaseResponse&, Response, bool, bIsBuy);

	UPROPERTY(BlueprintAssignable, Category = "Shop")
	FOnPurchaseResultReceived OnPurchaseResultReceived;

private:
	// 호스트/서버에서 실제 구매/판매를 수행하는 함수
	FPurchaseResponse PerformBuyItem(const FName& ItemId, int32 Quantity);
	FPurchaseResponse PerformSellItem(const FName& ItemId, int32 Quantity);

	// 상점에서 아이템 정보 조회 (서버 전용)
	bool FindShopItemByID(const FName& ItemId, FCMShopItemContent& OutItem) const;

	// 현재 상호작용 중인 NPC (상점 조회용)
	UPROPERTY()
	TWeakObjectPtr<ACMNpcBase> CurrentShopNpc;

	// 중복 구매 방지용 플래그
	bool bPurchaseInProgress = false;
	#pragma endregion

	#pragma region UI

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Interact")
	TObjectPtr<class UCMInteractionUIComponent> InteractionUIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Inventory")
	TObjectPtr<class UCMInventoryUIComponent> InventoryUIComponent;

	void SetInventoryInputMode(bool bIsInventoryOpen);

	UPROPERTY(EditDefaultsOnly, Category = "UI|SystemMenu")
	TSubclassOf<UUserWidget> SystemMenuWidgetClass;

private:
	void BindInteractionUI(APawn* InPawn);
	void InitializeInventoryUI(APawn* InPawn);

	virtual void OnSystemMenu() override;

	void OnInventoryClicked();

	void OnHelpAction();
	#pragma endregion

	#pragma region Character Spawn

public:
	// 클라이언트 준비 완료 시 서버에 선택된 캐릭터 태그를 보내 Pawn 스폰을 요청하는 함수
	UFUNCTION(BlueprintCallable, Category = "Player|Spawn")
	void NotifyServerPlayerReadyWithCharacter();

protected:
	// 서버 RPC: 선택된 캐릭터 태그를 기반으로 Pawn 스폰 및 빙의 처리
	UFUNCTION(Server, Reliable)
	void Server_SpawnPlayerWithSelectedCharacter(const FGameplayTag& SelectedCharacterTag);

	#pragma endregion


	#pragma region GameOver

public:
	// 게임 패배/승리 UI 표시 (로컬 플레이어 전용)
	UFUNCTION(BlueprintCallable, Category = "GameOver|UI")
	void ShowDefeatUI();

	UFUNCTION(BlueprintCallable, Category = "GameOver|UI")
	void ShowVictoryUI();

protected:
	// 에디터에서 지정할 수 있는 게임 패배/승리 UI 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameOver|UI")
	TSubclassOf<UUserWidget> DefeatWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameOver|UI")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	// 생성된 위젯 인스턴스 (로컬 전용)
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> DefeatWidgetInstance;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> VictoryWidgetInstance;

private:
	#pragma endregion

	#pragma region Character State Control

public:
	void HandlePlayerDeath();

private:
	#pragma endregion

	virtual void PostSeamlessTravel() override;
};