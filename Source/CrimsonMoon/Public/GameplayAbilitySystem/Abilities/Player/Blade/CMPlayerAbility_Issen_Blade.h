// Copyright CrimsonMoon Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_Issen_Blade.generated.h"

class UAnimMontage;
class UGameplayEffect;
class ACMProjectileActor;

/**
 * Blade 캐릭터의 차지 돌진 베기 스킬
 * - 입력 누름: 차지 시작
 * - 입력 릴리즈: 전방 고속 돌진 베기 실행 후 ApplyDamage
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_Issen_Blade : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_Issen_Blade();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	/** [Server RPC] 클라이언트에서 차지 릴리즈 요청 */
	UFUNCTION(Server, Reliable)
	void Server_ExecuteChargeSlash(float InChargeLevel);

	/** [Client RPC] 차지 레벨 동기화 및 일섬 실행 */
	UFUNCTION(Client, Reliable)
	void Client_ExecuteChargeSlash(float InChargeLevel);

private:
	/** 차지 시작 시 재생할 몽타주 (루프 애니메이션) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ChargeMontage;

	/** 돌진 베기 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ChargeSlashMontage;

	/** 타겟에게 적용할 데미지 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** 차지 최소 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinChargeTime = 0.3f;

	/** 차지 최대 시간 (초) - 최대 차지 도달 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxChargeTime = 2.0f;

	/** 돌진 거리 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashDistance = 1000.0f;

	/** 돌진 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashDuration = 0.3f;

	/** [신규] 최대 차지 시 그로기 데미지 배율 (예: 2.0배) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Groggy", meta = (AllowPrivateAccess = "true"))
	float MaxChargeGroggyMultiplier = 2.0f;

	/** 검기를 발사할 Projectile 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ACMProjectileActor> ProjectileClass;

	/** 검기 발사 차지 임계값 (이 값 이상이면 검기 발사, 미만이면 돌진) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float ChargeThreshold = 0.7f;

	/** 검기 최소 속도 (차지 임계값일 때) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinProjectileSpeed = 1500.0f;

	/** 검기 최대 속도 (100% 차지일 때) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxProjectileSpeed = 3000.0f;

	/** 검기 최대 비행 거리 (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ProjectileMaxDistance = 5000.0f;

	// 내부 상태 변수 (서버 관리)
	float CurrentChargeLevel = 0.0f; // 0.0 ~ 1.0

	float ChargeStartTime = 0.0f;
	bool bIsCharging = false;
	bool bIsDashing = false;

	// 내부 메서드
	void StartCharging();
	void ExecuteChargeSlash(float InChargeLevel);
	void PerformDash();
	void PerformProjectileSlash(float InChargeLevel) const;
	void OnDashComplete();

	// 몽타주 콜백
	UFUNCTION()
	void OnChargeMontageCompleted();

	UFUNCTION()
	void OnChargeSlashMontageCompleted();

	UFUNCTION()
	void OnChargeMontageCancelled();

	UFUNCTION()
	void OnMontageCancelled();

	// 데미지 이벤트 콜백 (서버 전용)
	UFUNCTION()
	void OnHitTarget(FGameplayEventData Payload);
};