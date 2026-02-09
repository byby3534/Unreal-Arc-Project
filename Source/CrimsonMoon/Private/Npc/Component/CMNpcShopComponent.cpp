// Fill out your copyright notice in the Description page of Project Settings.


#include "Npc/Component/CMNpcShopComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/CMPlayerController.h"
#include "Npc/CMNpcBase.h"

UCMNpcShopComponent::UCMNpcShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	NpcComponentType = ECMNpcComponentType::ShopComponent;
}

void UCMNpcShopComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCMNpcShopComponent::PerformAction()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			TEXT("UCMNpcShopComponent::PerformAction called - Shop action executed")
		);
	}
	UE_LOG(LogTemp, Log, TEXT("UCMNpcShopComponent::PerformAction"));

	// 상점 액션이 실행되면 서버에서 NPC ID 기반으로 상점 UI 오픈을 요청한다.
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 이 컴포넌트를 소유한 NPC에서 NpcId를 가져온다.
	ACMNpcBase* OwnerNpc = Cast<ACMNpcBase>(GetOwner());
	if (!OwnerNpc)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCMNpcShopComponent::PerformAction: Owner is not ACMNpcBase"));
		return;
	}

	const FName NpcId = OwnerNpc->NpcId;
	if (NpcId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("UCMNpcShopComponent::PerformAction: NpcId is None on %s"), *OwnerNpc->GetName());
		return;
	}

	// 로컬 플레이어 컨트롤러를 가져와 RequestOpenShopUI 호출
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return;
	}

	ACMPlayerController* CMPC = Cast<ACMPlayerController>(PC);
	if (!CMPC)
	{
		return;
	}

	CMPC->RequestOpenShopUI(NpcId);
}
