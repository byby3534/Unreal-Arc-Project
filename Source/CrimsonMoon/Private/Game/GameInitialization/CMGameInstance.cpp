// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameInitialization/CMGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Menus/CMCharacterSelectWidget.h"
#include "Components/UI/UIManagerComponent.h"
#include "Controllers/CMPlayerControllerBase.h"

#include "NavigationSystemTypes.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"

UCMGameInstance::UCMGameInstance()
{
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UCMGameInstance::OnCreateSessionComplete);
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UCMGameInstance::OnFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UCMGameInstance::OnJoinSessionComplete);
	// 초대 수락 전용 핸들러에 바인딩
	OnSessionUserInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UCMGameInstance::OnSessionUserInviteAccepted);
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UCMGameInstance::OnDestroySessionComplete);
}

void UCMGameInstance::Init()
{
	Super::Init();

	// 게임 시작 시 Steam 으로부터 닉네임을 가져와 저장
	InitPlayerNicknameFromSteam();

	// Steam 초대 수락 델리게이트 등록
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
				OnSessionUserInviteAcceptedDelegate
			);
		}
	}
}

void UCMGameInstance::TryJoinOrHostSession()
{
	if (bIsHosting)
	{
		HostGameTravel();
	}
	else
	{
		if (SessionInterfaceCache->JoinSession(LocalUserNumCache, NAME_GameSession, LastSessionSearchResult))
		{
			UE_LOG(LogTemp, Log, TEXT("OnSessionUserInviteAccepted: JoinSession 호출 성공"));
			SessionInterfaceCache.Reset();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OnSessionUserInviteAccepted: JoinSession 호출 실패"));
		}
	}
}

void UCMGameInstance::InitPlayerNicknameFromSteam()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitPlayerNicknameFromSteam: OnlineSubsystem not found"));
		return;
	}

	IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InitPlayerNicknameFromSteam: IdentityInterface invalid"));
		return;
	}

	const int32 LocalUserNum = 0;
	TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InitPlayerNicknameFromSteam: UserId invalid"));
		return;
	}

	const FString Nickname = IdentityInterface->GetPlayerNickname(*UserId);
	if (!Nickname.IsEmpty())
	{
		PlayerNickname = Nickname;
		UE_LOG(LogTemp, Log, TEXT("Steam Nickname initialized: %s"), *PlayerNickname);
	}
	else
	{
		PlayerNickname = GetPlayerNickname();
		UE_LOG(LogTemp, Warning, TEXT("InitPlayerNicknameFromSteam: Nickname empty"));
	}
}

void UCMGameInstance::CreateGameSession()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateGameSession: OnlineSubsystem not found"));
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateGameSession: SessionInterface invalid"));
		return;
	}

	// 이미 세션이 있으면 먼저 제거
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false; // Steam 은 false
	SessionSettings.bUsesPresence = true; // 존재 기반 매치 사용
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUseLobbiesIfAvailable = true; // UE 5.6 에서 Steam Lobby 사용
	SessionSettings.NumPublicConnections = 4; // 최대 인원
	SessionSettings.NumPrivateConnections = 0;
	SessionSettings.bAllowInvites = true;

	OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

	// 로컬 사용자 0 기준 (싱글 클라이언트)
	const int32 LocalUserNum = 0;
	const bool bResult = SessionInterface->CreateSession(LocalUserNum, NAME_GameSession, SessionSettings);

	if (bResult)
	{
		UE_LOG(LogTemp, Log, TEXT("CreateSession API 호출 성공"));
		bIsCreatedSession = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateSession API 호출 실패"));
	}
}

void UCMGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("세션 생성 성공: %s"), *SessionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("세션 생성 실패: %s"), *SessionName.ToString());
	}
}

void UCMGameInstance::HostGameTravel()
{
	// NOTE: ServerTravelToURL로 변경해야 하지만, 기존 TravelURL 변수를 그대로 사용하기 위해 별도 함수로 구현
	//		추후 TravelURL 변수를 제거하고 이 함수를 직접 호출하는 방식으로 변경 권장
	
	if (UWorld* World = GetWorld())
	{
		// TravelURL에 ?listen 옵션을 붙여서 리슨 서버로 동작하도록 구성
		FString FinalURL = TravelURL;

		// 이미 쿼리 파라미터가 있는지 확인 후 적절히 ? 또는 & 붙이기
		if (!FinalURL.Contains(TEXT("?")))
		{
			FinalURL += TEXT("?listen");
		}
		else
		{
			FinalURL += TEXT("&listen");
		}

		UE_LOG(LogTemp, Log, TEXT("HostGameTravel: ServerTravel to %s"), *FinalURL);
		World->ServerTravel(FinalURL);
	}
}

void UCMGameInstance::ServerTravelToURL(const FString& InURL)
{
	if (UWorld* World = GetWorld())
	{
		FString FinalURL = InURL;

		// 서버 트래블 시 항상 리슨 서버로 만들고 싶다면 InURL에 listen 파라미터를 보장
		if (!FinalURL.Contains(TEXT("?")))
		{
			FinalURL += TEXT("?listen");
		}
		else if (!FinalURL.Contains(TEXT("listen")))
		{
			FinalURL += TEXT("&listen");
		}

		UE_LOG(LogTemp, Log, TEXT("ServerTravelToURL: ServerTravel to %s"), *FinalURL);
		World->ServerTravel(FinalURL);
	}
}

void UCMGameInstance::FindGameSessions()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindGameSessions: OnlineSubsystem not found"));
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("FindGameSessions: SessionInterface invalid"));
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false; // Steam
	SessionSearch->MaxSearchResults = 100;
	// SEARCH_PRESENCE 매크로 대신 FName 키 직접 사용
	SessionSearch->QuerySettings.Set(FName(TEXT("SEARCH_PRESENCE")), true, EOnlineComparisonOp::Equals);

	OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

	const int32 LocalUserNum = 0;
	if (!SessionInterface->FindSessions(LocalUserNum, SessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Warning, TEXT("FindGameSessions: FindSessions 호출 실패"));
	}
}

void UCMGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	}

	if (!bWasSuccessful || !SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete: 검색 실패"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("OnFindSessionsComplete: Found %d sessions"), SessionSearch->SearchResults.Num());
}

void UCMGameInstance::JoinGameSession()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("JoinGameSession: OnlineSubsystem not found"));
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("JoinGameSession: SessionInterface invalid"));
		return;
	}

	if (!SessionSearch.IsValid() || SessionSearch->SearchResults.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("JoinGameSession: 유효한 검색 결과가 없습니다. 먼저 FindGameSessions를 호출하세요."));
		return;
	}

	// 일단 첫 번째 세션에 참가
	const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[0];

	// 여기서는 더 이상 델리게이트를 등록하지 않음 (Init 에서 한 번만 등록)

	const int32 LocalUserNum = 0;
	if (!SessionInterface->JoinSession(LocalUserNum, NAME_GameSession, Result))
	{
		UE_LOG(LogTemp, Warning, TEXT("JoinGameSession: JoinSession 호출 실패"));
	}
}

void UCMGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	// Init 에서 한 번만 등록해두고, 여기선 굳이 Clear 하지 않음 (Steam 초대 등 반복 Join 에 대응)

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete: Join failed (%d)"), (int32)Result);
		return;
	}

	// 세션에서 받은 접속 주소로 클라이언트 이동
	FString ConnectString;
	if (SessionInterface.IsValid() && SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
		{
			PC->ClientTravel(ConnectString, TRAVEL_Absolute);
			UE_LOG(LogTemp, Warning, TEXT("JoinGameSession: Connected to %s"), *ConnectString);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete: GetResolvedConnectString 실패"));
	}
}

void UCMGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, int32 ControllerId,
	TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogTemp, Log, TEXT("OnSessionUserInviteAccepted: Success=%d"), bWasSuccessful);

	if (!bWasSuccessful)
	{
		return;
	}

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		return;
	}

	SessionInterfaceCache = OnlineSub->GetSessionInterface();
	if (!SessionInterfaceCache.IsValid())
	{
		return;
	}

	LocalUserNumCache = ControllerId; // 보통 0, 초대 수락한 로컬 컨트롤러 기준
	bWasSuccessfulCache = bWasSuccessful;
	LastSessionSearchResult = InviteResult;
	bIsHosting = false;

	SessionInterfaceCache->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UCMGameInstance::OnJoinSessionComplete));

	// 여기서는 바로 세션에 조인하지 않고, 먼저 캐릭터 선택 UI를 표시한다.
	ShowCharacterSelectUI();
}

UCMCharacterSelectWidget* UCMGameInstance::ShowCharacterSelectUI()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 에디터에서 설정된 캐릭터 선택 위젯 클래스가 없으면 로깅 후 종료
	if (!CharacterSelectWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowCharacterSelectUI: CharacterSelectWidgetClass is not set on GameInstance."));
		return nullptr;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return nullptr;
	}

	ACMPlayerControllerBase* CMPC = Cast<ACMPlayerControllerBase>(PC);
	if (!CMPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowCharacterSelectUI: PlayerController is not ACMPlayerControllerBase."));
		return nullptr;
	}

	UUIManagerComponent* UIManager = CMPC->GetUIManagerComponent();
	if (!UIManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowCharacterSelectUI: UIManagerComponent is null."));
		return nullptr;
	}

	// GameInstance에서 지정한 캐릭터 선택 위젯 클래스로 UI 스택에 푸시
	if (UUserWidget* CreatedWidget = UIManager->PushWidget(CharacterSelectWidgetClass))
	{
		return Cast<UCMCharacterSelectWidget>(CreatedWidget);
	}

	UE_LOG(LogTemp, Warning, TEXT("ShowCharacterSelectUI: Failed to create CharacterSelectWidget via UIManager."));
	return nullptr;
}

void UCMGameInstance::LeaveGame()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// 현재 존재하는 세션이 있는지 확인 (NAME_GameSession은 기본 세션 이름)
			FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
			if (ExistingSession)
			{
				// 세션 파괴 완료 콜백 등록
				OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

				// 세션 파괴 요청 (스팀 로비 삭제)
				if (!SessionInterface->DestroySession(NAME_GameSession))
				{
					// 요청 실패 시 강제로 이동
					SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
					ReturnToMainMenu();
				}
				// 성공 시 OnDestroySessionComplete가 호출될 때까지 대기
				return;
			}
		}
	}

	// 세션이 없다면(싱글 플레이 등) 바로 이동
	ReturnToMainMenu();
}

void UCMGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnDestroySessionComplete: %s, Success: %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// 델리게이트 바인딩 해제
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		}
	}

	ReturnToMainMenu();
}

void UCMGameInstance::ReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}