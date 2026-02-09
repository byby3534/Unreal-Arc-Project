// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"
#include "CMPlayerGameplayAbility.generated.h"

class ACMPlayerCharacterBase;
class UCMItemInstance;
class UCMDataAsset_ConsumableData;

/**
 * 공통된 인벤토리/아이템 처리 로직을 포함
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerGameplayAbility : public UCMGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CMAbility|Helpers")
	ACMPlayerCharacterBase* GetCMPlayerCharacterFromActorInfo() const;

protected:

	 // 현재 어빌리티 실행과 연관된 아이템 인스턴스를 찾음
	UFUNCTION(BlueprintPure, Category = "CMAbility|Inventory")
	UCMItemInstance* GetAssociatedItemInstance() const;

	
	// 아이템을 찾아 수량을 감소시키고, 인벤토리 시스템에 알림
	UFUNCTION(BlueprintCallable, Category = "CMAbility|Inventory")
	bool FindAndConsumeItem(const UCMDataAsset_ConsumableData* TargetData, int32 Amount = 1);

	// 손에 아이템 메쉬 부착 헬퍼
	void AttachMeshToHand(const UCMDataAsset_ConsumableData* InConsumableData);

	// 손에서 아이템 메쉬 제거 헬퍼
	void DetachMeshFromHand();
};