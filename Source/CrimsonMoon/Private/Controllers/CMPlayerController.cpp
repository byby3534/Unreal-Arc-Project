// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/CMPlayerController.h"

#include "Components/UI/UIManagerComponent.h"
#include "Game/CMLocalEventManager.h"
#include "Net/UnrealNetwork.h"
#include "UI/NpcDialogue/CMNpcDialogueWidget.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Input/CMPickUpComponent.h"
#include "Components/UI/CMInteractionUIComponent.h"
#include "UI/Shop/CMShopWidget.h"
#include "Game/GameInitialization/CMNpcWorldSubsystem.h"
#include "Npc/CMNpcBase.h"
#include "Npc/Component/CMNpcShopComponent.h"
#include "Game/CMGameModeMainStage.h"
#include "GameplayTags/CMGameplayTags_Character.h"
#include "GameplayTags/CMGameplayTags_Currency.h"
#include "Game/GameInitialization/CMGameInstance.h"
#include "Components/UI/CMInventoryUIComponent.h"
#include "Components/CMInventoryComponent.h"
#include "Components/Input/CMInputComponent.h"
#include "CMGameplayTags.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "DataAssets/Input/CMDataAsset_InputConfig.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "Blueprint/UserWidget.h"
#include "Game/CMPlayerState.h"
#include "Game/CMGameStateMainStageV2.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/HUD/CMPlayerHUD.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Character/CMCharacterBase.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"

void ACMPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACMPlayerController, TeamId);
}

void ACMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	InitializeNpcDialogueEvent();

	// PlayerController 준비 완료 시 서버에 캐릭터 스폰을 요청
	NotifyServerPlayerReadyWithCharacter();
}

ACMPlayerController::ACMPlayerController()
{
	// UI Components
	InteractionUIComponent = CreateDefaultSubobject<UCMInteractionUIComponent>(TEXT("InteractionUIComponent"));
	InventoryUIComponent = CreateDefaultSubobject<UCMInventoryUIComponent>(TEXT("InventoryUIComponent"));
	TeamId = static_cast<uint8>(ETeamType::Player);
}

void ACMPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	BindInteractionUI(InPawn);

	InitializeInventoryUI(InPawn);
}

void ACMPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	APawn* CurrentPawn = GetPawn();
	BindInteractionUI(CurrentPawn);
	InitializeInventoryUI(CurrentPawn);
}

void ACMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UCMInputComponent* CMInput = Cast<UCMInputComponent>(InputComponent);
	if (!CMInput)
	{
		return;
	}

	if (InputConfigUIDataAsset)
	{
		CMInput->BindNativeInputAction(InputConfigUIDataAsset, CMGameplayTags::InputTag_UI_Inventory, ETriggerEvent::Started, this, &ThisClass::OnInventoryClicked);
		CMInput->BindNativeInputAction(InputConfigUIDataAsset, CMGameplayTags::InputTag_UI_Help, ETriggerEvent::Started, this, &ThisClass::OnHelpAction);
	}
}

FGenericTeamId ACMPlayerController::GetGenericTeamId() const
{
	return FGenericTeamId(TeamId);
}

void ACMPlayerController::SetTeamId(ETeamType NewTeamType)
{
	// 이 함수는 서버에서만 호출되어야 함
	if (!HasAuthority())
	{
		return;
	}
	TeamId = static_cast<uint8>(NewTeamType);
}

void ACMPlayerController::CreateNpcDialogueWidget()
{
	// 로컬 플레이어 컨트롤러 가져오기
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	ACMPlayerControllerBase* CMPC = Cast<ACMPlayerControllerBase>(PC);
	if (!CMPC)
	{
		return;
	}

	if (!IsValid(NpcDialogueWidgetClass))
	{
		return;
	}

	if (!CMPC->GetUIManagerComponent())
	{
		return;
	}

	// 이미 위젯이 있으면 다시 생성하지 않음
	if (IsValid(NpcDialogueWidgetInstance))
	{
		return;
	}

	if (UUserWidget* NewWidget = CMPC->GetUIManagerComponent()->PushWidget(NpcDialogueWidgetClass))
	{
		NpcDialogueWidgetInstance = Cast<UCMNpcDialogueWidget>(NewWidget);
	}
}

void ACMPlayerController::RemoveNpcDialogueWidget()
{
	// 현재 최상단 위젯이 이 NPC의 대화 위젯이라고 가정하고 Pop
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	ACMPlayerControllerBase* CMPC = Cast<ACMPlayerControllerBase>(PC);
	if (!CMPC || !CMPC->GetUIManagerComponent())
	{
		return;
	}

	// 위젯 인스턴스가 유효하면 PopWidget 으로 제거
	if (IsValid(NpcDialogueWidgetInstance))
	{
		CMPC->GetUIManagerComponent()->PopWidget(true);
		NpcDialogueWidgetInstance = nullptr;
	}
}

void ACMPlayerController::OnNextDialogueNodeRequested(const FText& DialogueText)
{
	if (!IsValid(NpcDialogueWidgetInstance))
	{
		CreateNpcDialogueWidget();
		NpcDialogueWidgetInstance->SetDialogueText(DialogueText);
	}
	else
	{
		NpcDialogueWidgetInstance->SetDialogueText(DialogueText);
	}
}

void ACMPlayerController::InitializeNpcDialogueEvent()
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
		if (LocalEventManager)
		{
			LocalEventManager->OnPrintDialogueNodeText.AddDynamic(this, &ACMPlayerController::OnNextDialogueNodeRequested);
			LocalEventManager->OnEndDialogueNodeRequested.AddDynamic(this, &ACMPlayerController::RemoveNpcDialogueWidget);
			UE_LOG(LogTemp, Log, TEXT("InitializeNpcDialogueEvent: LocalEventManager initialized in %s"), *GetName());
		}
	}
}

void ACMPlayerController::CreateShopWidget(const TArray<FCMShopItemContent>& InShopItems)
{
	// 이미 떠 있는 상점 위젯이 있다면 먼저 정리해서 스택/상태 꼬임 방지
	if (IsValid(ShopWidgetInstance))
	{
		UWorld* World = GetWorld();
		APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		if (PC)
		{
			ACMPlayerControllerBase* CMPC = Cast<ACMPlayerControllerBase>(PC);
			if (CMPC && CMPC->GetUIManagerComponent())
			{
				CMPC->GetUIManagerComponent()->PopWidget(true);
			}
		}

		ShopWidgetInstance = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateShopWidget: World is null on %s"), *GetName());
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateShopWidget: PlayerController is null"));
		return;
	}

	ACMPlayerControllerBase* CMPC = Cast<ACMPlayerControllerBase>(PC);
	if (!CMPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateShopWidget: PlayerController is not ACMPlayerControllerBase"));
		return;
	}

	if (!IsValid(ShopWidgetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateShopWidget: ShopWidgetClass is not set on %s"), *GetName());
		return;
	}

	if (!CMPC->GetUIManagerComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateShopWidget: UIManagerComponent is null on PlayerController"));
		return;
	}

	// UIManagerComponent 를 통해 상점 위젯을 스택에 Push
	if (UUserWidget* NewWidget = CMPC->GetUIManagerComponent()->PushWidget(ShopWidgetClass))
	{
		ShopWidgetInstance = Cast<UCMShopWidget>(NewWidget);
		if (ShopWidgetInstance)
		{
			ShopWidgetInstance->SetShopItems(InShopItems);
			ShopWidgetInstance->BuildShopList();
		}
	}
}

void ACMPlayerController::RequestOpenShopUI(const FName& NpcId)
{
	// 클라이언트에서 서버로 상점 데이터 요청
	if (!IsLocalController())
	{
		return;
	}

	Server_RequestShopData(NpcId);
}

void ACMPlayerController::RequestBuyItem(const FName& ItemId, int32 Quantity)
{
	if (!IsLocalController())
	{
		return;
	}

	// 호스트(리슨 서버)인 경우 바로 Perform 호출
	if (GetNetMode() == NM_ListenServer && HasAuthority())
	{
		const FPurchaseResponse Response = PerformBuyItem(ItemId, Quantity);
		// 호스트는 직접 결과 처리
		Client_ReceivePurchaseResult_Implementation(Response, true);
	}
	else
	{
		// 클라이언트인 경우 서버 RPC 호출
		Server_RequestBuyItem(ItemId, Quantity);
	}
}

void ACMPlayerController::RequestSellItem(const FName& ItemId, int32 Quantity)
{
	if (!IsLocalController())
	{
		return;
	}

	// 호스트(리슨 서버)인 경우 바로 Perform 호출
	if (GetNetMode() == NM_ListenServer && HasAuthority())
	{
		const FPurchaseResponse Response = PerformSellItem(ItemId, Quantity);
		// 호스트는 직접 결과 처리
		Client_ReceivePurchaseResult_Implementation(Response, false);
	}
	else
	{
		// 클라이언트인 경우 서버 RPC 호출
		Server_RequestSellItem(ItemId, Quantity);
	}
}

void ACMPlayerController::Server_RequestSellItem_Implementation(const FName& ItemId, int32 Quantity)
{
	// 서버에서 실제 판매 수행
	const FPurchaseResponse Response = PerformSellItem(ItemId, Quantity);

	// 결과를 클라이언트에게 전송
	Client_ReceivePurchaseResult(Response, false);
}

void ACMPlayerController::Server_RequestBuyItem_Implementation(const FName& ItemId, int32 Quantity)
{
	// 서버에서 실제 구매 수행
	const FPurchaseResponse Response = PerformBuyItem(ItemId, Quantity);

	// 결과를 클라이언트에게 전송
	Client_ReceivePurchaseResult(Response, true);
}

void ACMPlayerController::Client_ReceivePurchaseResult_Implementation(const FPurchaseResponse& Response, bool bIsBuy)
{
	const FString ActionType = bIsBuy ? TEXT("구매") : TEXT("판매");

	if (Response.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("[상점] %s 결과 수신: 성공 - %s x%d, 잔액=%d"),
			*ActionType, *Response.ItemID.ToString(), Response.Quantity, Response.RemainingCurrency);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] %s 결과 수신: 실패 - %s, 코드=%d"),
			*ActionType, *Response.ItemID.ToString(), static_cast<int32>(Response.Result));
	}

	// UI 피드백을 위한 델리게이트 브로드캐스트
	if (OnPurchaseResultReceived.IsBound())
	{
		OnPurchaseResultReceived.Broadcast(Response, bIsBuy);
	}
}

FPurchaseResponse ACMPlayerController::PerformBuyItem(const FName& ItemId, int32 Quantity)
{
	UE_LOG(LogTemp, Log, TEXT("[상점] PerformBuyItem: ItemId=%s, Quantity=%d"), *ItemId.ToString(), Quantity);

	// 1. 수량 검증
	if (Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: 잘못된 수량 %d"), Quantity);
		return FPurchaseResponse(EPurchaseResult::InvalidQuantity, ItemId, Quantity);
	}

	// 2. 중복 구매 방지
	if (bPurchaseInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: 이미 거래 진행 중"));
		return FPurchaseResponse(EPurchaseResult::ServerError, ItemId, Quantity);
	}
	bPurchaseInProgress = true;

	// RAII 스타일 락 해제
	ON_SCOPE_EXIT { bPurchaseInProgress = false; };

	// 3. 상점 아이템 조회
	FCMShopItemContent ShopItem;
	if (!FindShopItemByID(ItemId, ShopItem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: 아이템을 찾을 수 없음 - %s"), *ItemId.ToString());
		return FPurchaseResponse(EPurchaseResult::InvalidProduct, ItemId, Quantity);
	}

	// 4. ItemDataAsset 유효성 검사
	if (!ShopItem.ItemDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: ItemDataAsset이 설정되지 않음 - %s"), *ItemId.ToString());
		return FPurchaseResponse(EPurchaseResult::InvalidProduct, ItemId, Quantity);
	}

	// 5. 총 가격 계산
	const int32 TotalPrice = ShopItem.BuyPrice * Quantity;

	// 6. 플레이어 캐릭터 및 ASC 획득
	const ACMPlayerCharacterBase* PlayerCharacter = Cast<ACMPlayerCharacterBase>(GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: 플레이어 캐릭터를 찾을 수 없음"));
		return FPurchaseResponse(EPurchaseResult::ServerError, ItemId, Quantity, TotalPrice);
	}

	UCMAbilitySystemComponent* ASC = PlayerCharacter->GetCMAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: ASC를 찾을 수 없음"));
		return FPurchaseResponse(EPurchaseResult::ServerError, ItemId, Quantity, TotalPrice);
	}

	// 7. 재화 확인 (CanAfford)
	if (!ASC->CanAfford(TotalPrice))
	{
		const int32 CurrentCurrency = ASC->GetCurrentCurrencyAmount();
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: 재화 부족. 필요=%d, 보유=%d"), TotalPrice, CurrentCurrency);
		return FPurchaseResponse(EPurchaseResult::InsufficientFunds, ItemId, Quantity, TotalPrice, CurrentCurrency);
	}

	// 8. 인벤토리 컴포넌트 획득
	UCMInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UCMInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: 인벤토리 컴포넌트를 찾을 수 없음"));
		return FPurchaseResponse(EPurchaseResult::ServerError, ItemId, Quantity, TotalPrice);
	}

	// 9. 트랜잭션 실행 (재화 차감 -> 아이템 지급)
	// 9-1. 재화 차감 (GameplayEffect 사용)
	bool bSpendSuccess = false;
	ASC->SpendCurrency(TotalPrice, CMGameplayTags::Currency_Reason_Purchase, bSpendSuccess);

	if (!bSpendSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 구매 실패: 재화 차감 실패"));
		return FPurchaseResponse(EPurchaseResult::ServerError, ItemId, Quantity, TotalPrice);
	}

	// 9-2. 아이템 지급
	const int32 RemainingItems = InventoryComponent->AddItem(ShopItem.ItemDataAsset, Quantity);

	// 일부만 지급된 경우 (인벤토리 공간 부족)
	if (RemainingItems > 0)
	{
		const int32 AddedQuantity = Quantity - RemainingItems;
		const int32 RefundAmount = ShopItem.BuyPrice * RemainingItems;

		// 환불 처리
		ASC->AddCurrency(RefundAmount, CMGameplayTags::Currency_Reason_Purchase);

		UE_LOG(LogTemp, Warning, TEXT("[상점] 부분 구매: %d개 중 %d개만 지급됨. 환불액=%d"), Quantity, AddedQuantity, RefundAmount);

		if (AddedQuantity == 0)
		{
			return FPurchaseResponse(EPurchaseResult::InventoryFull, ItemId, 0, 0, ASC->GetCurrentCurrencyAmount());
		}

		return FPurchaseResponse(EPurchaseResult::Success, ItemId, AddedQuantity, ShopItem.BuyPrice * AddedQuantity, ASC->GetCurrentCurrencyAmount());
	}

	// 10. 성공
	const int32 RemainingCurrency = ASC->GetCurrentCurrencyAmount();
	UE_LOG(LogTemp, Log, TEXT("[상점] 구매 성공: %s x%d, 총가격=%d, 잔액=%d"), *ItemId.ToString(), Quantity, TotalPrice, RemainingCurrency);

	return FPurchaseResponse(EPurchaseResult::Success, ItemId, Quantity, TotalPrice, RemainingCurrency);
}

FPurchaseResponse ACMPlayerController::PerformSellItem(const FName& ItemId, int32 Quantity)
{
	UE_LOG(LogTemp, Log, TEXT("[상점] PerformSellItem: ItemId=%s, Quantity=%d"), *ItemId.ToString(), Quantity);

	// 1. 수량 검증
	if (Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 판매 실패: 잘못된 수량 %d"), Quantity);
		return FPurchaseResponse(EPurchaseResult::InvalidQuantity, ItemId, Quantity);
	}

	// 2. 중복 거래 방지
	if (bPurchaseInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 판매 실패: 이미 거래 진행 중"));
		return FPurchaseResponse(EPurchaseResult::ServerError, ItemId, Quantity);
	}
	bPurchaseInProgress = true;

	ON_SCOPE_EXIT { bPurchaseInProgress = false; };

	// 3. 상점 아이템 조회 (판매 가격 확인용)
	FCMShopItemContent ShopItem;
	if (!FindShopItemByID(ItemId, ShopItem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 판매 실패: 상점에서 아이템을 찾을 수 없음 - %s"), *ItemId.ToString());
		return FPurchaseResponse(EPurchaseResult::InvalidProduct, ItemId, Quantity);
	}

	// 4. 판매 가격이 0이면 판매 불가
	if (ShopItem.SellPrice <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 판매 실패: 판매 불가 아이템 - %s"), *ItemId.ToString());
		return FPurchaseResponse(EPurchaseResult::InvalidProduct, ItemId, Quantity);
	}

	// 5. ItemDataAsset 유효성 검사
	if (!ShopItem.ItemDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 판매 실패: ItemDataAsset이 설정되지 않음 - %s"), *ItemId.ToString());
		return FPurchaseResponse(EPurchaseResult::InvalidProduct, ItemId, Quantity);
	}

	// 6. 플레이어 캐릭터 및 컴포넌트 획득
	ACMCharacterBase* PlayerCharacter = Cast<ACMCharacterBase>(GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 판매 실패: 플레이어 캐릭터를 찾을 수 없음"));
		return FPurchaseResponse(EPurchaseResult::ServerError, ItemId, Quantity);
	}

	UCMAbilitySystemComponent* ASC = PlayerCharacter->GetCMAbilitySystemComponent();
	UCMInventoryComponent* InventoryComp = PlayerCharacter->FindComponentByClass<UCMInventoryComponent>();

	if (!ASC || !InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 판매 실패: ASC 또는 인벤토리를 찾을 수 없음"));
		return FPurchaseResponse(EPurchaseResult::ServerError, ItemId, Quantity);
	}

	// 7. 인벤토리에서 아이템 제거
	const bool bRemoved = InventoryComp->RemoveItem(ShopItem.ItemDataAsset, Quantity);
	if (!bRemoved)
	{
		UE_LOG(LogTemp, Warning, TEXT("[상점] 판매 실패: 인벤토리에 아이템이 없거나 수량 부족 - %s"), *ItemId.ToString());
		return FPurchaseResponse(EPurchaseResult::ItemNotOwned, ItemId, Quantity);
	}

	// 8. 재화 지급
	const int32 TotalPrice = ShopItem.SellPrice * Quantity;
	ASC->AddCurrency(TotalPrice, CMGameplayTags::Currency_Reason_Purchase);

	// 9. 성공
	const int32 RemainingCurrency = ASC->GetCurrentCurrencyAmount();
	UE_LOG(LogTemp, Log, TEXT("[상점] 판매 성공: %s x%d, 총가격=%d, 잔액=%d"),
		*ItemId.ToString(), Quantity, TotalPrice, RemainingCurrency);

	return FPurchaseResponse(EPurchaseResult::Success, ItemId, Quantity, TotalPrice, RemainingCurrency);
}

bool ACMPlayerController::FindShopItemByID(const FName& ItemId, FCMShopItemContent& OutItem) const
{
	if (!CurrentShopNpc.IsValid())
	{
		return false;
	}

	const UCMNpcShopComponent* ShopComponent = CurrentShopNpc->FindComponentByClass<UCMNpcShopComponent>();
	if (!ShopComponent)
	{
		return false;
	}

	TArray<FCMShopItemContent> ShopItems;
	ShopComponent->GetShopItemContents(ShopItems);

	for (const FCMShopItemContent& Item : ShopItems)
	{
		if (Item.ItemID == ItemId)
		{
			OutItem = Item;
			return true;
		}
	}

	return false;
}

bool ACMPlayerController::Server_RequestShopData_Validate(const FName& NpcId)
{
	// TODO: NPC와의 거리를 검사하여 유효한 요청인지 확인하는 로직 추가 필요
	return true;
}

void ACMPlayerController::Server_RequestShopData_Implementation(const FName& NpcId)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UCMNpcWorldSubsystem* NpcWorldSubsystem = World->GetSubsystem<UCMNpcWorldSubsystem>();
	if (!NpcWorldSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_RequestShopData: NpcWorldSubsystem not found"));
		return;
	}

	ACMNpcBase* FoundNpc = NpcWorldSubsystem->GetNpcById(NpcId);
	if (!FoundNpc)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_RequestShopData: NPC with Id %s not found"), *NpcId.ToString());
		return;
	}

	// NPC 의 ShopComponent 에서 상점 아이템 배열 가져오기
	UCMNpcShopComponent* ShopComp = FoundNpc->FindComponentByClass<UCMNpcShopComponent>();
	if (!ShopComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_RequestShopData: NPC %s has no ShopComponent"), *FoundNpc->GetName());
		return;
	}

	// 현재 상호작용 중인 NPC 저장 (구매/판매 시 아이템 조회용)
	CurrentShopNpc = FoundNpc;

	TArray<FCMShopItemContent> ShopItems;
	ShopComp->GetShopItemContents(ShopItems);

	if (World->GetNetMode() == NM_ListenServer && IsLocalController())
	{
		// 리슨 서버의 로컬 플레이어인 경우 바로 생성
		CreateShopWidget(ShopItems);
	}
	else
	{
		// 서버가 클라이언트에게 상점 데이터 전송
		Client_ReceiveShopDataAndOpen(ShopItems);
	}
}

void ACMPlayerController::Client_ReceiveShopDataAndOpen_Implementation(const TArray<FCMShopItemContent>& ShopItems)
{
	CreateShopWidget(ShopItems);
}

void ACMPlayerController::RemoveShopWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	// 현재 최상단 위젯이 상점 위젯이라고 가정하고 Pop
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	ACMPlayerControllerBase* CMPC = Cast<ACMPlayerControllerBase>(PC);
	if (!CMPC || !CMPC->GetUIManagerComponent())
	{
		return;
	}

	if (IsValid(ShopWidgetInstance))
	{
		CMPC->GetUIManagerComponent()->PopWidget(true);
		ShopWidgetInstance = nullptr;
	}
}

void ACMPlayerController::SetInventoryInputMode(bool bIsInventoryOpen)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem || !InputConfigUIDataAsset)
	{
		return;
	}

	ACMPlayerCharacterBase* PlayerCharacter = Cast<ACMPlayerCharacterBase>(GetPawn());
	UCMDataAsset_InputConfig* CharacterInputConfig = PlayerCharacter ? PlayerCharacter->GetInputConfigDataAsset() : nullptr;

	UCMDataAsset_InputConfig* PCInputConfig = InputConfigUIDataAsset;
	if (bIsInventoryOpen)
	{
		// 캐릭터 입력 차단
		if (PlayerCharacter)
		{
			PlayerCharacter->SetGameplayInputEnabled(false);
		}

		if (PCInputConfig && PCInputConfig->UIMappingContext)
		{
			Subsystem->AddMappingContext(PCInputConfig->UIMappingContext, 3);
		}
	}
	else
	{
		if (PCInputConfig && PCInputConfig->UIMappingContext)
		{
			Subsystem->RemoveMappingContext(PCInputConfig->UIMappingContext);
		}

		if (PlayerCharacter)
		{
			PlayerCharacter->SetGameplayInputEnabled(true);
		}
	}
}

void ACMPlayerController::InitializeInventoryUI(APawn* InPawn)
{
	if (!IsLocalPlayerController() || !InPawn)
	{
		return;
	}

	ACMPlayerCharacterBase* PlayerCharacter = Cast<ACMPlayerCharacterBase>(InPawn);
	if (!PlayerCharacter)
	{
		return;
	}

	UCMInventoryComponent* InventoryComp = PlayerCharacter->FindComponentByClass<UCMInventoryComponent>();
	UCMQuickBarComponent* QuickBarComp = PlayerCharacter->FindComponentByClass<UCMQuickBarComponent>();
	if (InventoryComp && QuickBarComp && InventoryUIComponent)
	{
		InventoryUIComponent->InitializeInventoryUI(InventoryComp, QuickBarComp);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeInventoryUI: %s 에서 InventoryComponent를 찾을 수 없습니다."), *InPawn->GetName());
	}
}

void ACMPlayerController::BindInteractionUI(APawn* InPawn)
{
	if (!InPawn || !IsLocalPlayerController())
	{
		return;
	}

	ACMPlayerCharacterBase* PlayerCharacter = Cast<ACMPlayerCharacterBase>(InPawn);
	if (!PlayerCharacter)
	{
		return;
	}

	UCMPickUpComponent* InteractionComponent = PlayerCharacter->FindComponentByClass<UCMPickUpComponent>();

	if (InteractionComponent && InteractionUIComponent)
	{
		const FName FuncName = FName("OnInteractableActorFound");
		if (!InteractionComponent->OnInteractableFound.Contains(InteractionUIComponent, FuncName))
		{
			InteractionComponent->OnInteractableFound.AddDynamic(InteractionUIComponent, &UCMInteractionUIComponent::OnInteractableActorFound);
		}
	}
}

void ACMPlayerController::OnSystemMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	if (IsValid(SystemMenuWidgetClass))
	{
		if (UUIManagerComponent* UIManager = GetUIManagerComponent())
		{
			UIManager->PushWidget(SystemMenuWidgetClass);
		}
	}
}

void ACMPlayerController::OnInventoryClicked()
{
	if (!IsLocalController())
	{
		return;
	}

	if (InventoryUIComponent)
	{
		InventoryUIComponent->OpenInventoryUI();
	}
}

void ACMPlayerController::OnHelpAction()
{
	// 현재 화면에 떠 있는 CMPlayerHUD 클래스의 위젯을 찾습니다.
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UCMPlayerHUD::StaticClass());

	if (FoundWidgets.Num() > 0)
	{
		if (UCMPlayerHUD* PlayerHUD = Cast<UCMPlayerHUD>(FoundWidgets[0]))
		{
			PlayerHUD->ToggleHelpWidget();
		}
	}
}

void ACMPlayerController::ShowDefeatUI()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!DefeatWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowDefeatUI: DefeatWidgetClass is not set on %s"), *GetName());
		return;
	}

	// 이미 떠 있으면 다시 생성하지 않음
	if (IsValid(DefeatWidgetInstance))
	{
		return;
	}

	if (UUIManagerComponent* UIManager = GetUIManagerComponent())
	{
		if (UUserWidget* NewWidget = UIManager->PushWidget(DefeatWidgetClass))
		{
			DefeatWidgetInstance = NewWidget;
		}
	}
}

void ACMPlayerController::ShowVictoryUI()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!VictoryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowVictoryUI: VictoryWidgetClass is not set on %s"), *GetName());
		return;
	}

	// 이미 떠 있으면 다시 생성하지 않음
	if (IsValid(VictoryWidgetInstance))
	{
		return;
	}

	if (UUIManagerComponent* UIManager = GetUIManagerComponent())
	{
		if (UUserWidget* NewWidget = UIManager->PushWidget(VictoryWidgetClass))
		{
			VictoryWidgetInstance = NewWidget;
		}
	}
}

void ACMPlayerController::NotifyServerPlayerReadyWithCharacter()
{
	// 로컬 컨트롤러에서만 호출
	if (!IsLocalController())
	{
		return;
	}

	// GameInstance에서 이미 지정된 SelectedCharacterTag를 가져와 사용
	FGameplayTag SelectedCharacterTag;

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GIBase = World->GetGameInstance())
		{
			if (UCMGameInstance* CMGI = Cast<UCMGameInstance>(GIBase))
			{
				SelectedCharacterTag = CMGI->GetSelectedCharacterTag();
			}
		}
	}

	// 서버에 Pawn 스폰 요청 (태그는 서버 RPC 인자로 전달)
	Server_SpawnPlayerWithSelectedCharacter(SelectedCharacterTag);
}

void ACMPlayerController::Server_SpawnPlayerWithSelectedCharacter_Implementation(const FGameplayTag& SelectedCharacterTag)
{
	// 서버 권한에서만 처리
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AGameModeBase* GMBase = World->GetAuthGameMode();
	ACMGameModeMainStage* CMGameMode = Cast<ACMGameModeMainStage>(GMBase);
	if (!CMGameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_SpawnPlayerWithSelectedCharacter - GameMode is not ACMGameModeMainStage"));
		return;
	}

	// 기존 코드: 즉시 스폰
	// CMGameMode->SpawnPlayerForController(this, SelectedCharacterTag);

	// 1초 뒤에 스폰되도록 타이머 설정
	FTimerHandle SpawnTimerHandle;
	World->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		FTimerDelegate::CreateLambda([CMGameMode, PC = TWeakObjectPtr<ACMPlayerController>(this), SelectedCharacterTag]() {
			if (!CMGameMode || !PC.IsValid())
			{
				return;
			}
			CMGameMode->SpawnPlayerForController(PC.Get(), SelectedCharacterTag);
		}),
		1.0f,
		false
		);
}

void ACMPlayerController::HandlePlayerDeath()
{
	// 서버에서만 플레이어 사망 처리
	if (!HasAuthority())
	{
		return;
	}

	// 이 컨트롤러에 연결된 PlayerState를 가져와서 사망 처리
	if (ACMPlayerState* CMPS = GetPlayerState<ACMPlayerState>())
	{
		CMPS->SetIsPlayerDead(true);
	}

	if (ACMGameStateMainStageV2* GS = GetWorld() ? GetWorld()->GetGameState<ACMGameStateMainStageV2>() : nullptr)
	{
		GS->IncrementDeathCount();
	}
}

void ACMPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// 로컬 컨트롤러가 아닌 경우 아무 것도 하지 않음 (서버 전용 PC 등 제외)
	if (!IsLocalController())
	{
		return;
	}

	// 1) 로컬 월드의 모든 InstancedStaticMeshComponent 정리
	/*
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor))
			{
				continue;
			}

			TArray<UInstancedStaticMeshComponent*> ISMComponents;
			Actor->GetComponents<UInstancedStaticMeshComponent>(ISMComponents);

			for (UInstancedStaticMeshComponent* ISMComp : ISMComponents)
			{
				if (!ISMComp)
				{
					continue;
				}

				// 레이트레이싱/충돌 문제를 방지하기 위해 모든 인스턴스를 제거
				ISMComp->ClearInstances();
				ISMComp->MarkRenderStateDirty();
			}
		}
	}
	*/

	// 2) 심리스 트래블 후 캐릭터 재스폰 요청
	NotifyServerPlayerReadyWithCharacter();

	// 3) 화면/사운드 페이드 인 (검정 → 정상)
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(
			/*FromAlpha*/ 1.0f,
			/*ToAlpha*/ 0.0f,
			/*Duration*/ 1.0f,
			/*Color*/ FLinearColor::Black,
			/*bShouldFadeAudio*/ true,
			/*bHoldWhenFinished*/ false
			);
	}
}