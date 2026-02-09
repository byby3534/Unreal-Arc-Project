// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Structs/CMStructTypes.h"
#include "GameplayTagContainer.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "CMDataAsset_WeaponData.generated.h"

class UCMPlayerLinkedAnimLayer;
class UInputMappingContext;
class UNiagaraSystem;
class ACMWeaponBase;
class UGameplayEffect;
class UGameplayAbility;
class UStaticMesh;

UCLASS()
class CRIMSONMOON_API UCMDataAsset_WeaponData : public UCMDataAsset_ItemBase
{
	GENERATED_BODY()

public:
	// 무기를 장착했을때 재생시킬 애님BP
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UCMPlayerLinkedAnimLayer> WeaponAnimLayerToLink;

	// 입력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> WeaponInputMappingContext;

	// 장비의 어빌리티
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (TitleProperty = "InputTag"))
	TArray<FCMPlayerAbilitySet> DefaultWeaponAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FScalableFloat WeaponBaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Groggy")
	FScalableFloat HeavyAttackGroggyDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Groggy")
	FScalableFloat CounterAttackGroggyDamage;

	// 무기가 소횐되는 소켓
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName EquippedSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Socket")
	FName UnequippedSocketName;

	// 이 무기의 스킬 애니메이션 몽타주에서 사용하는 나이아가라 파티클들
	// 무기 장착 시 미리 로드하여 스킬 첫 사용 시 프리징 방지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TArray<TSoftObjectPtr<UNiagaraSystem>> SkillParticleSystems;

	// 장착 시 캐릭터에게 부착될 '장착' 액터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equip")
	TSubclassOf<ACMWeaponBase> WeaponActorClass;

	// 이 무기를 장착하려고 할 때 발생시킬 이벤트 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	FGameplayTag EquipAbilityTriggerTag;

	// 습득 시 캐릭터에게 적용할 Gameplay Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TSubclassOf<UGameplayEffect> EquipGameplayEffect;
  
	// 이 무기를 장착할 수 있는 캐릭터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Class", meta = (Categories = "Character.Class"))
	FGameplayTag RequiredCharacterClassTag;
};
