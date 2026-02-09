// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/CMInteractableInterface.h"
#include "CMPickupActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UCMDataAsset_WeaponData;
class UCMDataAsset_ConsumableData;
class UCMDataAsset_ItemBase;

UCLASS()
class CRIMSONMOON_API ACMPickupActor : public AActor, public ICMInteractableInterface
{
	GENERATED_BODY()
	
public:	
	ACMPickupActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 인터페이스 구현
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FInteractionUIData GetInteractableData_Implementation() override;

	// 외부에서 데이터를 주입하는 함수
	void SetItemData(UCMDataAsset_ItemBase* NewItemData, int32 NewQuantity);

	// 아이템이 드랍될 때 튀어 나가는 연출을 실행하는 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDropEffect(FVector Impulse, FVector Torque);

protected:
	UFUNCTION()
	void OnRep_ItemData();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 상호작용 감지용 범위
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	// 아이템 데이터 베이스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemData, Category = "Item Data")
	TObjectPtr<UCMDataAsset_ItemBase> ItemData = nullptr;

	// 획득 수량 (기본값 1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Item Data")
	int32 ItemQuantity = 1;

private:
	// 물리 시뮬레이션을 끄기 위한 내부 함수
	void DisablePhysics();
};
