// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Pickups/CMPickupActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "Components/CMInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"

ACMPickupActor::ACMPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true); // 움직임 동기화 활성화

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 메쉬는 물리를 켜거나 막는 용도로 사용
	MeshComponent->SetCollisionProfileName(TEXT("BlockAll")); 
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetSimulatePhysics(false);

	// 상호작용 스피어
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(150.0f);

	// 충돌 설정: Pawn하고만 Overlap 되도록 설정
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);
}

void ACMPickupActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 분기문 없이 바로 설정
	if (ItemData && ItemData->PickupMesh)
	{
		MeshComponent->SetStaticMesh(ItemData->PickupMesh);
		MeshComponent->SetRelativeScale3D(ItemData->PickupMeshScale);
	}
}

void ACMPickupActor::BeginPlay()
{
	Super::BeginPlay();

	// BeginPlay 시점에서도 메쉬 업데이트 시도 (클라이언트에서 늦게 복제될 경우 대비)
	if (ItemData && ItemData->PickupMesh)
	{
		MeshComponent->SetStaticMesh(ItemData->PickupMesh);
		MeshComponent->SetRelativeScale3D(ItemData->PickupMeshScale);
	}
}

void ACMPickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACMPickupActor, ItemData);
	DOREPLIFETIME(ACMPickupActor, ItemQuantity);
}

FInteractionUIData ACMPickupActor::GetInteractableData_Implementation()
{
	FInteractionUIData Data;
	
	if (ItemData)
	{
		Data.TargetName = ItemData->ItemName;
		Data.ActionName = FText::FromString(TEXT("줍기"));
		Data.InputKeyText = FText::FromString(TEXT("2")); 
		Data.InteractionType = EInteractionType::Instant;
		
		// 수량 표시
		if (ItemQuantity > 1)
		{
			Data.TargetName = FText::Format(FText::FromString(TEXT("{0} (x{1})")), ItemData->ItemName, ItemQuantity);
		}
	}
	
	return Data;
}

void ACMPickupActor::SetItemData(UCMDataAsset_ItemBase* NewItemData, int32 NewQuantity)
{
	if (HasAuthority())
	{
		ItemData = NewItemData;
		ItemQuantity = NewQuantity;

		// 서버에서도 메쉬 업데이트
		OnRep_ItemData();
	}
}

void ACMPickupActor::Multicast_PlayDropEffect_Implementation(FVector Impulse, FVector Torque)
{
	if (!MeshComponent)
	{
		return;
	}

	// 물리 시뮬레이션 켜기
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 서버에서 계산된 힘과 토크 적용
	MeshComponent->AddImpulse(Impulse, NAME_None, true);
	MeshComponent->AddTorqueInDegrees(Torque, NAME_None, true);

	// 일정 시간 후 물리 끄기 (안정화)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACMPickupActor::DisablePhysics, 2.0f, false);
}

void ACMPickupActor::OnRep_ItemData()
{
	if (ItemData && MeshComponent)
	{
		if (ItemData->PickupMesh)
		{
			MeshComponent->SetStaticMesh(ItemData->PickupMesh);
			MeshComponent->SetRelativeScale3D(ItemData->PickupMeshScale);
		}
	}
}

void ACMPickupActor::DisablePhysics()
{
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false);
		// 필요하다면 콜리전을 다시 OverlapOnly로 변경할 수도 있습니다.
		// MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ACMPickupActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || !Interactor)
	{
		return;
	}

	UCMInventoryComponent* Inventory = Interactor->FindComponentByClass<UCMInventoryComponent>();
	if (Inventory)
	{
		// 아이템을 넣고, 들어가지 못한 남은 수량을 받음
		int32 LeftoverAmount = Inventory->AddItem(ItemData, ItemQuantity);

		if (LeftoverAmount == 0)
		{
			// 픽업 액터 파괴
			Destroy();
		}
		else if (LeftoverAmount < ItemQuantity)
		{
			// 공간 부족 -> 픽업 액터의 수량을 남은 만큼 줄임
			ItemQuantity = LeftoverAmount;
			UE_LOG(LogTemp, Warning, TEXT("인벤토리 공간 부족. 남은 수량: %d"), ItemQuantity);
		}
		else
		{
			// 가득 참 -> 아무 일도 안 함
			UE_LOG(LogTemp, Warning, TEXT("인벤토리가 꽉 차서 주울 수 없습니다."));
		}
	}
}