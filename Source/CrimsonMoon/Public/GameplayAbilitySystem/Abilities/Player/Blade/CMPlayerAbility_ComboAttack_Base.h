// Copyright CrimsonMoon Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CMGameplayTags.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_ComboAttack_Base.generated.h"

/**
 * @class UCMPlayerAbility_ComboAttack_Base
 * @brief Blade 캐릭터의 콤보 공격 시스템 베이스 클래스 (추상 클래스)
 *
 * [공통 기능]
 * - 콤보 몽타주 순차 재생
 * - 카운터 어택 지원
 * - 데미지 처리
 * - 네트워크 복제 (리슨 서버 환경)
 *
 * [자식 클래스가 구현해야 할 기능]
 * - HandleComboComplete: 콤보 완료 시 처리 (리셋 타이머 등)
 * - HandleComboCancelled: 콤보 취소 시 처리 (즉시 리셋 등)
 *
 * [리슨 서버 고려]
 * 1. 콤보 상태(CurrentComboCount)는 서버에서만 변경하고 클라이언트로 복제(RPC)하여 동기화
 * 2. 스태미나 소모는 서버에서만 적용
 * 3. 데미지 적용은 서버의 충돌 판정(OnHitTarget)에 의해서만 트리거
 *
 * InstancedPerActor로 설정되어 콤보 상태를 유지
 */
UCLASS(Abstract)
class CRIMSONMOON_API UCMPlayerAbility_ComboAttack_Base : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_ComboAttack_Base();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	/**
	 * 콤보가 정상 완료되었을 때 호출 (자식 클래스가 구현)
	 * - BaseAttack: 리셋 타이머 시작
	 * - SkillCombo: 아무것도 안함 (리셋 없음)
	 */
	virtual void HandleComboComplete();

	/**
	 * 콤보가 취소되었을 때 호출 (자식 클래스가 구현)
	 * - BaseAttack: 즉시 리셋
	 * - SkillCombo: 즉시 리셋 또는 아무것도 안함
	 */
	virtual void HandleComboCancelled();

	/**
	 * 콤보 카운트를 리셋 (서버 전용)
	 * 자식 클래스에서 호출 가능
	 */
	void ResetComboCount();

private:
	/** 순차적으로 재생할 공격 몽타주 배열 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	/** 카운터 어택 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CounterAttackMontage;

	/** 타겟에게 적용할 데미지 게임플레이 이펙트 (서버 전용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** [리슨 서버] 현재 콤보 횟수 (서버에서 관리하며 RPC로 클라이언트에 동기화) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combo", meta = (AllowPrivateAccess = "true"))
	int32 CurrentComboCount = 0;

	/** 콤보 공격 타입 (Light/Heavy) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo|Attack", meta = (AllowPrivateAccess = "true", Categories = "Player.SetByCaller"))
	FGameplayTag ComboAttackTypeTag = CMGameplayTags::Player_SetByCaller_AttackType_Light;

	/** 몽타주 재생이 정상적으로 완료/블렌드아웃되었을 때 호출 */
	UFUNCTION()
	void OnMontageEnded();

	/** 몽타주가 취소되거나 중단되었을 때 호출 */
	UFUNCTION()
	void OnMontageCancelled();

	/** [서버 전용] 'Shared_Event_MeleeHit' 이벤트를 수신했을 때 호출 */
	UFUNCTION()
	void OnHitTarget(FGameplayEventData Payload);

	/** [서버 -> 클라] 콤보 카운트를 클라이언트에 동기화 */
	UFUNCTION(Client, Reliable)
	void Client_SyncComboCount(int32 NewComboCount);
};