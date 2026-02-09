// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Object/CMItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "Components/CMInventoryComponent.h"

UCMItemInstance::UCMItemInstance()
{
	Quantity = 1;
}

void UCMItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 변수들이 복제되도록 등록
	DOREPLIFETIME(UCMItemInstance, ItemData);
	DOREPLIFETIME(UCMItemInstance, Quantity);
}

void UCMItemInstance::OnRep_Quantity()
{
	UCMInventoryComponent* InventoryComp = nullptr;
	
	if (UCMInventoryComponent* OuterComp = Cast<UCMInventoryComponent>(GetOuter()))
	{
		InventoryComp = OuterComp;
	}
	else if (AActor* OuterActor = Cast<AActor>(GetOuter()))
	{
		InventoryComp = OuterActor->FindComponentByClass<UCMInventoryComponent>();
	}

	// 인벤토리 컴포넌트를 찾았다면 변경 알림 전송
	if (InventoryComp)
	{
		// UI 갱신
		InventoryComp->OnInventoryItemChanged.Broadcast(this);
		
		UE_LOG(LogTemp, Log, TEXT("[Client] Item Quantity Updated: %d"), Quantity);
	}
}
