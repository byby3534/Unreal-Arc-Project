// Fill out your copyright notice in the Description page of Project Settings.


#include "Npc/CMNpcGameStarter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Components/UI/UIManagerComponent.h"
#include "Controllers/CMPlayerControllerBase.h"

// Sets default values
ACMNpcGameStarter::ACMNpcGameStarter()
{
	bReplicates = true;
}

void ACMNpcGameStarter::Interact_Implementation(AActor* Interactor)
{
	// 반드시 서버에서만 처리
	if (!HasAuthority() && GetNetMode() != NM_ListenServer)
	{
		return;
	}

	// Remote Client가 조종 중인 Pawn에서 온 Interact는 무시
	if (APawn* Pawn = Cast<APawn>(Interactor))
	{
		if (AController* Controller = Pawn->GetController())
		{
			// 서버 입장에서 IsPlayerController && !IsLocalController 이면 Remote 클라
			if (Controller->IsPlayerController() && !Controller->IsLocalController())
			{
				return;
			}
		}
	}

	// 여기까지 왔다면 허용된 Interactor이므로 실제 GameStart 로직 수행
	PerformInteract();
}

void ACMNpcGameStarter::PerformInteract()
{
	// 서버 권한이 있는 경우에만 처리
	if (!HasAuthority() && GetNetMode() != NM_ListenServer)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 모든 클라이언트에 페이드 아웃 시작 요청
	MulticastStartFadeOut();

	// 페이드가 끝난 뒤에 ServerTravel 실행
	FTimerHandle TravelTimerHandle;
	World->GetTimerManager().SetTimer(
		TravelTimerHandle,
		this,
		&ACMNpcGameStarter::ServerTravelToConfiguredMap,
		FadeDuration,
		false
	);
}

void ACMNpcGameStarter::MulticastStartFadeOut_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (PC->IsLocalController())
		{
			PC->PlayerCameraManager->StartCameraFade(
				/*FromAlpha*/			0.0f,
				/*ToAlpha*/				1.0f,
				/*Duration*/			FadeDuration,
				/*Color*/				FLinearColor::Black,
				/*bShouldFadeAudio*/	true,
				/*bHoldWhenFinished*/	false
			);

			if (ACMPlayerControllerBase* CMPC = Cast<ACMPlayerControllerBase>(PC))
			{
				CMPC->GetUIManagerComponent()->ResetUI();
			}
		}
	}
}

void ACMNpcGameStarter::ServerTravelToConfiguredMap()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	if (!HasAuthority())
	{
		return;
	}

	if (TravelURL.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ACMNpcGameStarter::ServerTravelToConfiguredMap - TravelURL is empty"));
		return;
	}

	const bool bAbsolute = false;
	World->SeamlessTravel(TravelURL, /*bAbsolute*/ bAbsolute);
}