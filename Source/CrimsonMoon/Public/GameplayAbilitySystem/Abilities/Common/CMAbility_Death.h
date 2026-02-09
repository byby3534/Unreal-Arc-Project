// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"
#include "CMAbility_Death.generated.h"

/**
 * 플레이어와 AI 공통 사망 어빌리티
 * - 서버에서만 실행
 * - 캐릭터의 NetMulticast RPC를 통해 모든 클라이언트에서 사망 애니메이션 재생
 * - 캐릭터 비활성화 및 콜리전 처리
 */
UCLASS()
class CRIMSONMOON_API UCMAbility_Death : public UCMGameplayAbility
{
	GENERATED_BODY()

public:
	UCMAbility_Death();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 사망 애니메이션 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Animation")
	TArray<TObjectPtr<UAnimMontage>> DeathMontages;

	/** 사망 후 캐릭터를 파괴하기까지의 지연 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Config")
	float DestroyDelay = 5.0f;

private:
	/** 파괴 타이머 핸들 (취소 가능하도록 멤버 변수로 저장) */
	FTimerHandle DestroyTimerHandle;

	/** 서버에서만 실행: 캐릭터 비활성화 처리 */
	void HandleDeathOnServer();

	/** 모든 클라이언트에서 실행: 시각적 효과 처리 */
	void HandleDeathCosmetics();

	/** 타이머에서 호출: 캐릭터 파괴 처리 */
	UFUNCTION()
	void HandleCharacterDestruction();
};