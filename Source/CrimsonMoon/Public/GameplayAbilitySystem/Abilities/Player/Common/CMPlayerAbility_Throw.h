// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_Throw.generated.h"

class UCMDataAsset_ConsumableData;

/**
 *  투척형 아이템 사용 어빌리티
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_Throw : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_Throw();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
		) override;

protected:
	// 몽타주 재생이 끝나거나 취소 되었을때 처리
	UFUNCTION()
	void OnMontageCompleted();

	// 이벤트 수신 함수
	UFUNCTION()
	void OnThrowEventReceived(FGameplayEventData Payload);

	// 실제 발사체 스폰 로직 (클라/서버 모두 호출)
	void SpawnProjectile();

	// 데이터 에셋 캐싱
	UPROPERTY()
	TObjectPtr<const UCMDataAsset_ConsumableData> ConsumableData;

	// 투척 시점을 감지할 태그
	UPROPERTY(EditDefaultsOnly, Category = "Throw")
	FGameplayTag ThrowEventTag;
};
