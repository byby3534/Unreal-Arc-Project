// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Items/Enums/CMItemTier.h"
#include "CMDataAsset_ArmorData.generated.h"

class UCMPlayerLinkedAnimLayer;
class ACMWeaponBase;
class UGameplayEffect;
class UGameplayAbility;
class AItemPickup;

// 방어구 아이템 부위
UENUM(BlueprintType)
enum class EArmorType : uint8
{
	Helmet		UMETA(DisplayName = "Helmet"),
	Plate		UMETA(DisplayName = "Armor"),
	Chest		UMETA(DisplayName = "Chest"),
	Tasset		UMETA(DisplayName = "Tasset"),
	Greaves		UMETA(DisplayName = "Greaves"),
	
};

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMDataAsset_ArmorData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

#pragma region BaseInformation
	
	// 아이템 이름 (UI 표시용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
	FText ItemName;

	// 아이템 아이콘 (UI 표시용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
	TSoftObjectPtr<UTexture2D> ItemIcon;

	// 아이템 툴팁 등에 표시될 상세 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
	FText ItemDescription;

	// 아이템 희귀도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Tier")
	ECMItemTier ItemTier;

	// 아이템 유형 (흉갑, 팔 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type")
	EArmorType ArmorType;

#pragma endregion

#pragma region SlotAndTags

	// 이 아이템이 장착될 슬롯을 나타내는 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Slot")
	FGameplayTag ArmorSlotTag;

	// 이 아이템을 설명하는 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Slot")
	FGameplayTagContainer ItemTags;

#pragma endregion

#pragma region World

	// 캐릭터에 부착될 실제 외형 액터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<ACMWeaponBase> AttachedActorClass;

	// 방어구가 소횐되는 소켓
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName EquippedSocketName;

#pragma endregion

#pragma region GAS

	// 착용 시 소유자에게 적용할 게임플레이 이펙트 (스탯 부여용, Duration: Infinite)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> EquipEffect;

	// 착용 시 소유자에게 부여할 게임플레이 어빌리티 (액티브 스킬 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

#pragma endregion
};
