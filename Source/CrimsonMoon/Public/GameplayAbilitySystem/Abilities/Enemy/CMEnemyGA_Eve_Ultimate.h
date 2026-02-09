// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility.h"
#include "GameplayTags/CMGameplayTags_Shared.h"
#include "CMEnemyGA_Eve_Ultimate.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMEnemyGA_Eve_Ultimate : public UCMEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UCMEnemyGA_Eve_Ultimate();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 궁극기 시전 동작 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "CrimsonMoon|Config")
	TObjectPtr<UAnimMontage> UltimateMontage;

	/** AnimNotify로 전달받을 폭발 트리거 태그 */
	UPROPERTY(EditDefaultsOnly, Category = "CrimsonMoon|Config")
	FGameplayTag ExplosionEventTag = CMGameplayTags::Shared_Event_Attack;

	UPROPERTY(EditDefaultsOnly, Category = "CrimsonMoon|Config")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 보스가 이동할 맵 중앙 위치를 표시하는 액터의 태그 */
	UPROPERTY(EditDefaultsOnly, Category = "CrimsonMoon|Config")
	FName ArenaCenterActorTag = FName("BossArenaCenter");

	/** 바닥에 깔릴 경고 장판 GameplayCue 태그 (예: GameplayCue.Enemy.Eve.Ultimate.Warning) */
	UPROPERTY(EditDefaultsOnly, Category = "CrimsonMoon|Config")
	FGameplayTag WarningCueTag;

	/** 폭발 이펙트용 GameplayCue 태그 (예: GameplayCue.Enemy.Eve.Ultimate.Explode) */
	UPROPERTY(EditDefaultsOnly, Category = "CrimsonMoon|Config")
	FGameplayTag ExplosionCueTag;

	/* 스킬의 기본 데미지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CrimsonMoon|Config")
	float SkillBaseDamage = 999999.0f;

private:
	UFUNCTION()
	void OnExplosionTriggered(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnMontageCancelled();
};