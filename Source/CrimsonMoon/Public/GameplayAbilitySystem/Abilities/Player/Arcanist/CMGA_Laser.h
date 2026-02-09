// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "GameplayTags/CMGameplayTags_GameplayCue.h"
#include "CMGA_Laser.generated.h"

class UAnimMontage;
class UAbilityTask_WaitRepeat;

/**
 * 레이저 어빌리티
 * - Local Client: 카메라 트레이스로 목표 지점 계산 후 서버 전송
 * - Server: 데미지 처리 및 GameplayCue 파라미터(목표 지점) 전파
 */
UCLASS()
class CRIMSONMOON_API UCMGA_Laser : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMGA_Laser();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnTick();

	/** 몽타주 완료 콜백 */
	UFUNCTION()
	void OnMontageCompleted();

	/** 몽타주 취소 콜백 */
	UFUNCTION()
	void OnMontageCancelled();

	/** 레이저 시작 이벤트 콜백 */
	UFUNCTION()
	void OnLaserStartEvent(FGameplayEventData Payload);

	/** 클라이언트가 계산한 목표 지점을 서버로 전송 */
	UFUNCTION(Server, Reliable)
	void Server_ProcessLaserTick(const FVector& TargetHitLocation);

	/** 카메라 기준 트레이스 수행 (로컬에서 수행) */
	FVector GetCameraAimLocation() const;

	void ApplyDamageToServer(const FVector& TargetLocation);

	UPROPERTY(EditDefaultsOnly, Category = "Lazer|Damage")
	float DamageMultiplier = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Lazer|Damage")
	float DamageTickInterval = 0.2f;

	/** 레이저 판정 반지름 */
	UPROPERTY(EditDefaultsOnly, Category = "Lazer|Damage")
	float LaserTraceRadius = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	FName MuzzleSocketName = FName("Yuna_HandGrip_R");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Effects")
	TSubclassOf<UGameplayEffect> StartSoundEffectClass;

	FActiveGameplayEffectHandle StartSoundEffectHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CM|Effects")
	TSubclassOf<UGameplayEffect> LoopSoundEffectClass;

	FActiveGameplayEffectHandle LoopSoundEffectHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Lazer")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Lazer")
	float MaxLaserRange = 3000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Lazer")
	float LaserDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Lazer")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** 레이저 애니메이션 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Lazer|Animation")
	TObjectPtr<UAnimMontage> LaserMontage;

	/** 레이저 시작 이벤트 태그 (Start 섹션의 노티파이에서 발생) */
	UPROPERTY(EditDefaultsOnly, Category = "Lazer|Animation")
	FGameplayTag LaserStartEventTag;

	// 최적화: 거리 비교 상수를 미리 정의
	// 만약 5cm 움직임 체크라면 5*5=25
	static constexpr float MIN_ReplicationDistSquared = 25.0f;

private:
	/** 마지막으로 서버에 전송한 목표 위치 */
	FVector LastSentTargetLocation;

	/** 마지막으로 동기화된 목표 지점 */
	FVector LastSyncedLocation;
};