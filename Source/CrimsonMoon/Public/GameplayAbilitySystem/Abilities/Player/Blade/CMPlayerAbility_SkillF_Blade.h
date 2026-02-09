// Copyright CrimsonMoon Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_SkillF_Blade.generated.h"

class UAnimMontage;
class UGameplayEffect;

/**
 * Blade 캐릭터의 전방위 광역 베기 스킬 (F 키)
 * - 입력 시 1.5초간 차징
 * - 차징 완료 후 자동으로 전방위 광역 베기 실행
 * - 무기 콜리전(BoxComponent/SphereComponent) + AN_ToggleCollision으로 히트 감지
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_SkillF_Blade : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_SkillF_Blade();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/** 차징 시작 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ChargeStartMontage;

	/** 차징 루프 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ChargeLoopMontage;

	/** 광역 베기 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SlashMontage;

	/** 타겟에게 적용할 데미지 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** 차징 루프 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ChargeTime = 1.5f;

	// 내부 상태 변수
	bool bIsCharging = false;
	FTimerHandle ChargeLoopTimerHandle;


	UFUNCTION()
	void ExecuteSlash();

	UFUNCTION()
	void StartChargeLoop();

	UFUNCTION()
	void OnChargeStartMontageCompleted();

	UFUNCTION()
	void OnSlashMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnHitTarget(FGameplayEventData Payload);

	void StartCharging();
};