// Copyright CrimsonMoon Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_SoulSiphon.generated.h"

class UAnimMontage;
class UGameplayEffect;

/**
 * SoulSiphon R 스킬
 * - 캐릭터 중심으로 스피어 콜리전 생성
 * - VFX 시각 효과에 맞춰 주변 적들에게 단일 피해를 2번 적용
 * - VFX 시작 시 첫 번째 데미지
 * - VFX 끝나기 전 두 번째 데미지
 * - 피격된 적들에게 슬로우 효과 적용
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_SoulSiphon : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_SoulSiphon();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/** 스킬 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SkillMontage;

	/** 타겟에게 적용할 데미지 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** 타겟에게 적용할 슬로우 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> SlowEffect;

	/** 스킬 데미지 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float DamageMultiplier = 1.0f;

	/** 스킬 범위 (반경) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float SkillRadius = 600.0f;

	/** 슬로우 지속 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	float SlowDuration = 3.0f;

	/** 슬로우 강도 (이동 속도 감소 비율, 0.0 ~ 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float SlowIntensity = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> SoundEffectClass;

	FActiveGameplayEffectHandle SoundEffectHandle;

	/** 몽타주 종료 콜백 */
	UFUNCTION()
	void OnMontageCompleted();

	/** 몽타주 취소 콜백 */
	UFUNCTION()
	void OnMontageCancelled();

	/** 첫 번째 데미지 이벤트 콜백 (VFX 시작 시) */
	UFUNCTION()
	void OnFirstDamageTriggered(FGameplayEventData Payload);

	/** 두 번째 데미지 이벤트 콜백 (VFX 끝나기 전) */
	UFUNCTION()
	void OnSecondDamageTriggered(FGameplayEventData Payload);

	/** 범위 내 적들에게 데미지 및 슬로우 적용 */
	void ApplyAoEDamageAndSlow(bool bIsFirstDamage);
};