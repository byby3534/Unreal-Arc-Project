// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h" // EOnJoinSessionCompleteResult, FOnJoinSessionCompleteDelegate
#include "GameplayTagContainer.h"
#include "CMGameInstance.generated.h"

class UCMCharacterSelectWidget; // 캐릭터 선택 위젯 전방 선언
class APlayerController;

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMGameInstance : public UGameInstance
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	UCMGameInstance();
	
	virtual void Init() override;
	
	/* Steam Online Service Methods */
public:
	// 세션 생성 (Steam용)
	UFUNCTION(BlueprintCallable, Category="Online|Session")
	void CreateGameSession();

	UFUNCTION(BlueprintCallable, Category="Online|Session")
	void FindGameSessions();

	UFUNCTION(BlueprintCallable, Category="Online|Session")
	void JoinGameSession();

	void SetIsCreatedSession(bool bInIsCreated) { bIsCreatedSession = bInIsCreated; }
	bool GetIsCreatedSession() const { return bIsCreatedSession; }	

	// 세션을 이미 생성/보유하고 있다고 가정하고, 호스트가 실제 게임 맵으로 이동(리스너 서버 시작)
	UFUNCTION(BlueprintCallable, Category="Online|Session")
	void HostGameTravel();

	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void LeaveGame();

	// URL을 인자로 받아 해당 레벨로 서버 트래블하는 함수
	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void ServerTravelToURL(const FString& InURL);

	// 블루프린트에서 닉네임 읽기용 Getter
	UFUNCTION(BlueprintPure, Category="Online|Player")
	FString GetPlayerNickname() const { return PlayerNickname; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FGameplayTag GetSelectedCharacterTag() const { return SelectedCharacterTag; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetSelectedCharacterTag(FGameplayTag InTag) { SelectedCharacterTag = InTag; }

	UFUNCTION()
	FORCEINLINE void SetIsHosting(bool bInIsHosting) { bIsHosting = bInIsHosting; }
	
	UFUNCTION()
	void TryJoinOrHostSession();

	// --- 캐릭터 선택 UI 제어(GameInstance에서 구현, 컨트롤러는 참조만) ---
	// 에디터에서 지정한 캐릭터 선택 위젯 클래스를 사용해, 현재 로컬 PlayerController의 UIManager에 푸시한다.
	UFUNCTION(BlueprintCallable, Category="UI|CharacterSelect")
	UCMCharacterSelectWidget* ShowCharacterSelectUI();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category="Online|Player")
	FString PlayerNickname;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Online|Session")
	FString TravelURL;

	// GameInstance에서 사용할 캐릭터 선택 위젯 클래스 (에디터에서 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|CharacterSelect")
	TSubclassOf<UCMCharacterSelectWidget> CharacterSelectWidgetClass;

private:
	void InitPlayerNicknameFromSteam(); // Steam (OnlineSubsystem) 에서 현재 유저 닉네임을 읽어와 PlayerNickname에 저장

	// 세션 생성 완료 콜백 (이제 여기서는 ServerTravel을 하지 않음)
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, int32 ControllerId,
                                     TSharedPtr<const FUniqueNetId> UserId,
                                     const FOnlineSessionSearchResult& InviteResult);

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	TDelegate<void(FName, EOnJoinSessionCompleteResult::Type)> OnJoinSessionCompleteDelegate;
	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY()
	FGameplayTag SelectedCharacterTag;

	// 초대 수락 후 Join/Host를 지연시키기 위해 캐시된 값들
	IOnlineSessionPtr SessionInterfaceCache;
	bool bWasSuccessfulCache;
	int32 LocalUserNumCache = -1;
	FOnlineSessionSearchResult LastSessionSearchResult;
	bool bIsHosting = true;
	
	// 세션 파괴 완료 시 호출될 콜백 함수
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// 실제 레벨 이동을 처리하는 함수
	void ReturnToMainMenu();
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	bool bIsCreatedSession = false;

	// --- 메인ステ이지 진행 상태 저장용 ---
	UPROPERTY()
	int32 SavedCurrentPlayerCount = 0;

	UPROPERTY()
	int32 SavedDeathCount = 0;

public:
	// GameState에서 현재 카운트를 저장
	UFUNCTION()
	void SaveMainStageCounts(int32 InCurrentPlayerCount, int32 InDeathCount)
	{
		SavedCurrentPlayerCount = InCurrentPlayerCount;
		SavedDeathCount = InDeathCount;
	}

	// GameState에서 카운트를 복원할 때 사용
	UFUNCTION()
	void LoadMainStageCounts(int32& OutCurrentPlayerCount, int32& OutDeathCount) const
	{
		OutCurrentPlayerCount = SavedCurrentPlayerCount;
		OutDeathCount = SavedDeathCount;
	}
};
