// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "Components/CMInventoryComponent.h"
#include "Items/Object/CMItemInstance.h"
#include "DataAssets/Consumable/CMDataAsset_ConsumableData.h"
#include "Components/StaticMeshComponent.h"

ACMPlayerCharacterBase* UCMPlayerGameplayAbility::GetCMPlayerCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ACMPlayerCharacterBase>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

UCMItemInstance* UCMPlayerGameplayAbility::GetAssociatedItemInstance() const
{
	// Spec의 SourceObject에서 시도
	if (UCMItemInstance* ItemInstance = Cast<UCMItemInstance>(GetSourceObject(CurrentSpecHandle, CurrentActorInfo)))
	{
		return ItemInstance;
	}

	// 실패 시 QuickBar에서 현재 활성 아이템 찾기 (Fallback)
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (UCMQuickBarComponent* QuickBar = Avatar->FindComponentByClass<UCMQuickBarComponent>())
		{
			return QuickBar->GetActiveSlotItem();
		}
	}

	return nullptr;
}

bool UCMPlayerGameplayAbility::FindAndConsumeItem(const UCMDataAsset_ConsumableData* TargetData, int32 Amount)
{
	// 서버 권한 확인
	if (!GetOwningActorFromActorInfo()->HasAuthority())
	{
		return false;
	}

	UCMItemInstance* ItemInstance = GetAssociatedItemInstance();

	// 인스턴스 존재 여부 확인
	if (!ItemInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FindAndConsumeItem] 실패: 아이템 인스턴스를 찾을 수 없습니다."));
		return false;
	}

	// 데이터 일치 여부 확인
	if (TargetData && ItemInstance->ItemData != TargetData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FindAndConsumeItem] 실패: 대상 데이터가 일치하지 않습니다."));
		return false;
	}

	// 수량 확인
	if (ItemInstance->Quantity < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FindAndConsumeItem] 실패: 수량이 부족합니다. (보유: %d, 필요: %d)"), ItemInstance->Quantity, Amount);
		return false;
	}

	// 인벤토리 컴포넌트에 제거 요청
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (UCMInventoryComponent* Inventory = Avatar->FindComponentByClass<UCMInventoryComponent>())
		{
			Inventory->RemoveItemInstance(ItemInstance, Amount);
			UE_LOG(LogTemp, Log, TEXT("[FindAndConsumeItem] 성공: %s (x%d) 소모 완료. 잔여: %d"), 
				*ItemInstance->ItemData->GetName(), Amount, ItemInstance->Quantity);
			return true;
		}
	}

	return false;
}

void UCMPlayerGameplayAbility::AttachMeshToHand(const UCMDataAsset_ConsumableData* InConsumableData)
{
	ACMPlayerCharacterBase* CMCharacter = GetCMPlayerCharacterFromActorInfo();
	if (CMCharacter && InConsumableData && InConsumableData->HandHeldMesh)
	{
		UStaticMeshComponent* HandMesh = CMCharacter->GetHandHeldItemMesh();
		if (HandMesh)
		{
			HandMesh->SetStaticMesh(InConsumableData->HandHeldMesh);
			HandMesh->SetWorldScale3D(InConsumableData->HandHeldMeshScale);
            
			// 소켓 이름 결정
			FName SocketName = InConsumableData->HandHeldSocketName.IsNone() ? FName("Hand_Item") : InConsumableData->HandHeldSocketName;
            
			if (HandMesh->GetAttachSocketName() != SocketName)
			{
				HandMesh->AttachToComponent(CMCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			}
            
			HandMesh->SetVisibility(true);
		}
	}
}

void UCMPlayerGameplayAbility::DetachMeshFromHand()
{
	ACMPlayerCharacterBase* CMCharacter = GetCMPlayerCharacterFromActorInfo();
	if (CMCharacter)
	{
		UStaticMeshComponent* HandMesh = CMCharacter->GetHandHeldItemMesh();
		if (HandMesh)
		{
			HandMesh->SetVisibility(false);
			HandMesh->SetStaticMesh(nullptr);
			HandMesh->SetWorldScale3D(FVector(1.0f));
		}
	}
}