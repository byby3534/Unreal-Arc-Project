// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "AnimInstance/Player/CMPlayerLinkedAnimLayer.h"
#include "GameplayAbilitySystem/CMAttributeSet.h"
#include "Components/SlateWrapperTypes.h"
#include "Enums/CMEnums.h"
#include "Engine/DataAsset.h"
#include "Misc/Guid.h"
#include "CMStructTypes.generated.h"

class ACMWeaponBase;
class UInputMappingContext;
class UCMPlayerGameplayAbility;
class UCMDataAsset_ConsumableData;
class UCMDataAsset_WeaponData;

USTRUCT(BlueprintType)
struct FCMInputActionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputAction> InputAction;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct FCMPlayerAbilitySet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	// UI 슬롯 태그 (예: InputTag_Skill_Q) - 캐릭터 클래스와 무관하게 UI 슬롯 매핑용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Player.Skill"))
	FGameplayTag SlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCMPlayerGameplayAbility> AbilityToGrant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SoftAbilityIconMaterial;

	// 쿨다운 태그 (GE에서 사용하는 CooldownTag)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

	bool IsValid() const;
};

// 리플리케이트되는 어빌리티 아이콘 데이터
USTRUCT(BlueprintType)
struct FAbilityIconData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotTag;

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag CooldownTag;

	FAbilityIconData() = default;

	FAbilityIconData(const FGameplayTag& InInputTag, const FGameplayTag& InSlotTag, const TSoftObjectPtr<UTexture2D>& InIcon, const FGameplayTag& InCooldownTag = FGameplayTag())
		: SlotTag(InSlotTag)
		, Icon(InIcon)
		, InputTag(InInputTag)
		, CooldownTag(InCooldownTag) {}
};

struct FCMDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageTaken)
	DECLARE_ATTRIBUTE_CAPTUREDEF(GroggyDamageTaken)

	FCMDamageCapture()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCMAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCMAttributeSet, DefensePower, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCMAttributeSet, DamageTaken, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCMAttributeSet, GroggyDamageTaken, Target, false);
	}
};

USTRUCT()
struct FReplicatedWeaponEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag WeaponTag;

	UPROPERTY()
	TObjectPtr<ACMWeaponBase> WeaponActor;

	// TArray 복제가 효율적으로 작동하기 위해 비교 연산자가 필요
	bool operator==(const FReplicatedWeaponEntry& Other) const
	{
		return WeaponTag == Other.WeaponTag && WeaponActor == Other.WeaponActor;
	}
};

USTRUCT(BlueprintType)
struct FInteractionUIData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText TargetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InputKeyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EInteractionType InteractionType = EInteractionType::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "InteractionType == EInteractionType::Hold"))
	float InteractionDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	float CurrentProgress = 0.0f;
};

USTRUCT(BlueprintType)
struct FWidgetConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget|Config")
	EUILayerType UILayer = EUILayerType::GameMenu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget|Config")
	EUIInputMode UIInputMode = EUIInputMode::GameAndUI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget|Config")
	bool bShowMouseCursor = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget|Config")
	ESlateVisibility PreviousWidgetVisibility = ESlateVisibility::HitTestInvisible;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget|Config")
	ESlateVisibility DefaultVisibility = ESlateVisibility::Visible; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget|Config")
	bool bShouldCache = false;
};
USTRUCT(BlueprintType)
struct FCMItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName;

	// 아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemDescription;

	// 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	// 실제로 월드에 스폰되거나 손에 쥘 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> Mesh;
	
	// 아이템 장착 시 부여할 어빌리티 태그 (무기인 경우)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	FGameplayTag EquipAbilityTag;

	// 아이템 사용 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<class UGameplayAbility> ActiveAbilityClass;

	// --- [스탯] ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 MaxStack = 1;
};