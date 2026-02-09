// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Enums/CMEnums.h"
#include "CMGameplayAbility.generated.h"

class UPawnCombatComponent;
class ACMCharacterBase;
class UCMAbilitySystemComponent;
/**
 *
 */
UCLASS()
class CRIMSONMOON_API UCMGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	#pragma region Native Overrides
	/** 어빌리티가 ASC에 부여될 때 자동으로 호출 */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	#pragma endregion

	#pragma region Helpers
	/** 액터 정보에서 UCMAbilitySystemComponent를 캐스팅하여 반환 */
	UFUNCTION(BlueprintPure, Category = "CMAbility | Helpers")
	UCMAbilitySystemComponent* GetCMAbilitySystemComponentFromActorInfo() const;

	/** 액터 정보에서 ACMCharacterBase를 캐스팅하여 반환 */
	UFUNCTION(BlueprintPure, Category = "CMAbility | Helpers")
	ACMCharacterBase* GetCMCharacterFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "CMAbility | Helpers")
	UPawnCombatComponent* GetPawnCombatComponentFromActorInfo() const;
	#pragma endregion

	#pragma region Attributes
	UFUNCTION(BlueprintPure, Category = "CMAbility | Attributes")
	float GetCurrentAttackSpeed() const;
	#pragma endregion

	#pragma region Cooldown
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	#pragma endregion

protected:
	#pragma region Policy
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CMAbility")
	ECMAbilityActivationPolicy AbilityActivationPolicy = ECMAbilityActivationPolicy::OnTriggered;
	#pragma endregion

	#pragma region Cooldown
	// 쿨타임 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown", Meta = (ClampMin = "0.0"))
	float CooldownDurationSeconds = 0.f;

	// 쿨타임 중임을 식별할 고유 태그
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown", Meta = (Categories = "Cooldown"))
	FGameplayTagContainer CooldownIdentifierTags;
	#pragma endregion

	#pragma region Damage Handling
	/* 데미지 GE 스펙을 생성하고 SetByCaller 태그를 설정하는 헬퍼 함수 (플레이어/적 모두 사용 가능) */
	UFUNCTION(BlueprintPure, Category = "CMAbility | Helpers")
	FGameplayEffectSpecHandle MakeDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass, float InWeaponBaseDamage, float InGroggyDamage, FGameplayTag InAttackTypeTag, int32 InComboCount) const;
	#pragma endregion

	#pragma region Gameplay Cue
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayCue", meta = (Categories = "GameplayCue"))
	FGameplayTag GameplayCueTag;

	/**
	 * 무기 공격 히트 시 GameplayCue VFX를 실행.
	 * @param HitActor 피격 대상 액터
	 * @param InGameplayCueTag 실행할 GameplayCue 태그 (지정하지 않으면 GameplayCueTag 프로퍼티 사용)
	 */
	UFUNCTION(BlueprintCallable, Category = "CMAbility | GameplayCue")
	void ExecuteWeaponHitGameplayCue(const AActor* HitActor, const FGameplayTag& InGameplayCueTag = FGameplayTag()) const;

	/**
	 * 타겟이 데미지를 받을 때 타겟의 ASC에서 GameplayCue를 실행.
	 * @param HitActor 피격 대상 액터
	 * @param InGameplayCueTag 실행할 GameplayCue 태그
	 */
	UFUNCTION(BlueprintCallable, Category = "CMAbility | GameplayCue")
	void ExecuteTargetHitGameplayCue(const AActor* HitActor, const FGameplayTag& InGameplayCueTag) const;
	#pragma endregion

private:
	#pragma region Combat Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = true))
	float CounterAttackDamageMultiplier = 2.0f;
	#pragma endregion
};