// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CMGameStateBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACMGameStateBase::ACMGameStateBase()
{
	//TODO: 동기화가 필요한 컴포넌트들을 여기에 등록
	SyncedComponentMap.Add(FName("GameState"), 0);
}

void ACMGameStateBase::StartClientLoadSequence()
{
	// NOTE: GameState/PlayerState 동기화 완료 이후 호출 기대
	
	UE_LOG(LogTemp, Log, TEXT("[Load] All Sync Complete, Start ClientLoadSequence"));
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACMGameStateBase::HandleLoadBegin));
	}
}

void ACMGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	NotifyComponentSynced(FName("GameState"));
}

void ACMGameStateBase::RegisterSyncedComponents()
{
	
}

void ACMGameStateBase::NotifyAllPlayerStateSynced()
{

}

void ACMGameStateBase::NotifyComponentSynced(FName ComponentName)
{
	// 컴포넌트가 동기화되었음을 알림
	if (SyncedComponentMap.Contains(ComponentName)) 
	{
		SyncedComponentMap[ComponentName] = 1;
		SyncedComponentCount++;
	}

	if (SyncedComponentCount >= SyncedComponentMap.Num()) 
	{
		// 모든 컴포넌트가 동기화되었을 때 로드 시퀀스 시작
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ClientLoadSequenceDelayTimerHandle,
				this,
				&ACMGameStateBase::StartClientLoadSequence,
				ClientLoadSequenceDelay,
				false);
		}
	}
}

void ACMGameStateBase::HandleLoadBegin()
{
	UE_LOG(LogTemp, Log, TEXT("[Load] HandleLoadBegin -> Broadcast"));
	OnLoadBegin.Broadcast();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACMGameStateBase::HandleLoadUI));
	}
}

void ACMGameStateBase::HandleLoadUI()
{
	UE_LOG(LogTemp, Log, TEXT("[Load] HandleLoadUI -> Broadcast"));
	OnLoadUI.Broadcast();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACMGameStateBase::HandleLoadCompleted));
	}
}

void ACMGameStateBase::HandleLoadCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[Load] HandleLoadCompleted -> Broadcast"));
	OnLoadCompleted.Broadcast();
}





