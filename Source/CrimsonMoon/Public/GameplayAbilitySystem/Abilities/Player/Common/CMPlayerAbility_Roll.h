// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_Roll.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UMotionWarpingComponent;

/**
 * 8방향 롤링 어빌리티
 * - 플레이어의 입력 방향에 따라 8방향 중 하나의 롤링 몽타주를 재생합니다.
 * - 모션 워핑을 사용하여 목표 위치로 캐릭터를 이동시킵니다.
 * - 라인 트레이스를 사용하여 롤링 거리를 계산하고 장애물을 감지합니다.
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_Roll : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_Roll();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** 비무장 상태 롤링 몽타주 맵 (방향별로 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TMap<ERollDirection, TObjectPtr<UAnimMontage>> RollMontages;

	/** 무기 장착 상태 Dodge 몽타주 맵 (방향별로 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TMap<ERollDirection, TObjectPtr<UAnimMontage>> DodgeMontages;

	/** 기본 롤링 거리 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll Settings")
	float DefaultRollDistance = 400.0f;

	/** 롤링 중 무적 여부를 결정하는 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> InvincibilityEffect;

	/** 모션 워핑 타겟 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Motion Warping")
	FName WarpTargetName = "RollTarget";

	/** 라인 트레이스에 사용할 오브젝트 타입 (WorldStatic, WorldDynamic 등) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll Settings")
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	/** 라인 트레이스 디버그 드로잉 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll Settings")
	bool bDebugDrawTrace = false;

private:
	/** 현재 재생 중인 몽타주 태스크 */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	/** 적용된 무적 이펙트 핸들 */
	FActiveGameplayEffectHandle InvincibilityEffectHandle;

	/** 몽타주 완료 콜백 */
	UFUNCTION()
	void OnMontageCompleted();

	/** 몽타주 취소/중단 콜백 */
	UFUNCTION()
	void OnMontageCancelled();

	/** 입력 벡터를 기반으로 롤링 방향 계산 */
	static ERollDirection CalculateRollDirection(const FVector2D& InputVector);

	/** 라인 트레이스를 사용하여 안전한 롤링 거리 계산 */
	float CalculateSafeRollDistance(const FVector& StartLocation, const FVector& Direction) const;

	/** 모션 워핑 타겟 설정 */
	void SetupMotionWarping(const FVector& TargetLocation) const;
};