// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_Block.generated.h"

class UAbilityTask_PlayMontageAndWait;
/**
 * 막기 및 패링 어빌리티
 * - 입력을 누르고 있으면 'Blocking' 상태가 됩니다. (데미지 감소는 AttributeSet에서 각도 검증 후 처리)
 * - 애니메이션 노티파이를 통해 'PerfectParryWindow' 태그를 짧은 시간 동안 부여합니다. (패링 판정용)
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_Block : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_Block();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> BlockStartMontage;

	/* 패링 가능 상태(Player.Status.PerfectParryWindow)를  짧은 시간(예: 0.2초)동안 부여하는 게임플레이 이펙트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> ParryWindowEffect;

	/* 퍼펙트 패링 성공 시 카운터 어택 가능 태그를 부여하는 게임플레이 이펙트 (Duration은 GE 에셋에서 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> CounterAttackWindowEffect;

	/* 퍼펙트 패링 성공 시 공격자에게 가할 그로기 데미지 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> ParryGroggyDamageEffect;

	/* 패링 이펙트를 적용할 시점을 알려주는 이벤트 태그 (몽타주 노티파이에서 호출) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event", meta = (AllowPrivateAccess = true))
	FGameplayTag ParryWindowStartEventTag; // 예: "Player.Event.ParryWindowStart"

	/* GameplayCue를 부착할 메시를 찾기 위한 태그 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameplayCue", meta = (AllowPrivateAccess = true))
	FName TargetMeshTagName;

	/* 활성화 시 적용된 ParryWindowEffect의 핸들 (종료 시 제거하기 위함) */
	FActiveGameplayEffectHandle ParryWindowEffectHandle;

	/* 현재 재생 중인 몽타주 태스크 */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	float OriginalMaxWalkSpeed;

	/* [추가] 몽타주가 끝났을 때 호출될 함수 (아무것도 안 함) */
	UFUNCTION()
	virtual void OnMontageEnded();

	/* 몽타주에서 패링 이벤트 태그를 수신했을 때 호출될 함수 */
	UFUNCTION()
	void OnParryWindowStart(FGameplayEventData Payload);

	UFUNCTION()
	void OnSuccessfulParry(FGameplayEventData Payload);

	UFUNCTION()
	void OnPerfectBlock();

	UFUNCTION()
	virtual void OnPerfectParrySuccess(FGameplayEventData Payload);

	FGameplayCueParameters MakeBlockGameplayCueParams() const;
};