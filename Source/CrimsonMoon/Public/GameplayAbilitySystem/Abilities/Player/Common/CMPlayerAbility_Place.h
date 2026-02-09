// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_Place.generated.h"

class UCMDataAsset_ConsumableData;

/**
 * 설치형 아이템 사용 어빌리티
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_Place : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_Place();

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
	UFUNCTION()
	void OnMontageCompleted();

	// 설치 이벤트(노티파이) 수신 시
	UFUNCTION()
	void OnPlaceEventReceived(FGameplayEventData Payload);

	// 실제 액터 스폰 및 아이템 소모
	UFUNCTION()
	void SpawnPlaceableActor();

	// 데이터 에셋 캐싱
	UPROPERTY()
	TObjectPtr<const UCMDataAsset_ConsumableData> ConsumableData;

	// 감지할 이벤트 태그
	UPROPERTY(EditDefaultsOnly, Category = "Place")
	FGameplayTag PlaceEventTag;
};
