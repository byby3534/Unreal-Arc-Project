// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Weapons/CMWeaponBase.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "CMPlayerWeapon.generated.h"

class UCMDataAsset_WeaponData;

UCLASS()
class CRIMSONMOON_API ACMPlayerWeapon : public ACMWeaponBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "WeaponData")
	TObjectPtr<UCMDataAsset_WeaponData> WeaponData;

	UFUNCTION(BlueprintCallable)
	void AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InSpecHandles);

	UFUNCTION(BlueprintPure)
	const TArray<FGameplayAbilitySpecHandle>& GetGrantedAbilitySpecHandles() const;

private:
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;

	// 적용된 스텟 이펙트 추가용 핸들
	FActiveGameplayEffectHandle EquipEffectHandle;
};