// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CMGameStateBase.generated.h"

// 3단계 로드 델리게이트 선언 (C++ 전용)
DECLARE_MULTICAST_DELEGATE(FOnLoadBegin);
DECLARE_MULTICAST_DELEGATE(FOnLoadUI);
DECLARE_MULTICAST_DELEGATE(FOnLoadCompleted);

/**
 *
 */
UCLASS()
class CRIMSONMOON_API ACMGameStateBase : public AGameState
{
	GENERATED_BODY()

public:
	ACMGameStateBase();
	
	FOnLoadBegin OnLoadBegin;
	FOnLoadUI OnLoadUI;
	FOnLoadCompleted OnLoadCompleted;

	virtual void NotifyComponentSynced(FName ComponentName);
	
	void StartClientLoadSequence();

protected:
	TMap<FName, int8> SyncedComponentMap;
	int32 SyncedComponentCount = 0;
	
	virtual void BeginPlay() override;
	
	virtual void NotifyAllPlayerStateSynced();
	
	virtual void HandleLoadBegin();
	virtual void HandleLoadUI();
	virtual void HandleLoadCompleted();

private:
	FTimerHandle ClientLoadSequenceDelayTimerHandle;
	float ClientLoadSequenceDelay = 1.0f;
	
	void RegisterSyncedComponents();
	
};