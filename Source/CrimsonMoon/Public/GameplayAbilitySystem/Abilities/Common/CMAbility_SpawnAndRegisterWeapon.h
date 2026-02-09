// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMAbility_SpawnAndRegisterWeapon.generated.h"

class ACMWeaponBase;
/**
 * [공용 어빌리티]
 * 서버에서 무기를 스폰하고 PawnCombatComponent에 등록
 * 이 어빌리티는 즉시 종료
 */
UCLASS()
class CRIMSONMOON_API UCMAbility_SpawnAndRegisterWeapon : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMAbility_SpawnAndRegisterWeapon();

	/* 어빌리티 활성화 시 서버에서만 실행 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/* 서버에서만 활성화되도록 제한합니다. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	/* 스폰할 무기 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<ACMWeaponBase> WeaponClassToSpawn;

	/* 스폰된 무기를 등록할 게임플레이 태그 (Equip GA에서 이 태그를 사용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FGameplayTag WeaponTagToRegister;

	/* 스폰 직후 무기를 부착할 초기 소켓 (예: Back_Socket) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName InitialSocketName;

	/* 스폰과 동시에 장착할지 여부 (true = 즉시 장착, false = 등록만) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bRegisterAsEquippedWeapon = false;

};