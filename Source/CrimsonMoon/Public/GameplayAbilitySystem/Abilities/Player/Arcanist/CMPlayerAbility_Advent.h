// Copyright CrimsonMoon Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_Advent.generated.h"

class UAnimMontage;
class UGameplayEffect;
class ACMAdventActor;

/**
 * Advent F 스킬
 * - 카메라 라인트레이스로 적 감지
 * - 천둥번개 타격 (초기 AoE 데미지)
 * - 장판 생성 (지속 도트 데미지)
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_Advent : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_Advent();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/** 스킬 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SkillMontage;

	/** 초기 폭발 데미지 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> InitialDamageEffect;

	/** 도트 데미지 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DotDamageEffect;

	/** 초기 폭발 데미지 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float InitialDamageMultiplier = 2.0f;

	/** 도트 데미지 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float DotDamageMultiplier = 0.5f;

	/** 폭발 범위 (반경) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ExplosionRadius = 500.0f;

	/** 도트 데미지 틱 간격 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float DotInterval = 0.5f;

	/** 장판 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float DotDuration = 5.0f;

	/** 라인트레이스 최대 거리 (라인트레이스 최대 거리 - 캐릭터 위치 = 스킬 범위 거리) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float MaxTraceDistance = 1000.0f;

	/** 라인트레이스 채널 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Camera;

	/** 디버그 드로우 활성화 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bEnableDebugDraw = false;

	/** 디버그 드로우 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true", EditCondition = "bEnableDebugDraw"))
	float DebugDrawDuration = 3.0f;

	/** 장판 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ACMAdventActor> AdventActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Effects", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> StartSoundEffectClass;

	FActiveGameplayEffectHandle StartSoundEffectHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Effects", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> ThunderSoundEffectClass;

	FActiveGameplayEffectHandle ThunderSoundEffectHandle;

	/** 캐시된 타겟 위치 */
	FVector CachedTargetLocation;

	/** 카메라 라인트레이스로 타겟 위치 계산 */
	FVector GetCameraTraceTarget() const;

	/** 서버에 타겟 위치 전달 */
	UFUNCTION(Server, Reliable)
	void ServerSetTargetLocation(FVector_NetQuantize TargetLocation);

	/** 몽타주 종료 콜백 */
	UFUNCTION()
	void OnMontageCompleted();

	/** 몽타주 취소 콜백 */
	UFUNCTION()
	void OnMontageCancelled();

	/** 천둥번개 타격 이벤트 콜백 */
	UFUNCTION()
	void OnThunderStrikeEvent(FGameplayEventData Payload);

	/** 장판 액터 소환 콜백 */
	UFUNCTION()
	void OnAdventActorSpawned(ACMProjectileActor* SpawnedActor);

	/** 초기 폭발 데미지 적용 */
	void ApplyInitialExplosionDamage(const FVector& Location);
};