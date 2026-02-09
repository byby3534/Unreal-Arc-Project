// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"
#include "CMAbility_Groggy.generated.h"

/**
 * 그로기 상태를 관리하는 Gameplay Ability (플레이어/적 공용)
 * Shared.Event.GroggyTriggered 이벤트로 활성화
 */
UCLASS()
class CRIMSONMOON_API UCMAbility_Groggy : public UCMGameplayAbility
{
	GENERATED_BODY()

public:
	UCMAbility_Groggy();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** 그로기 상태를 부여하는 Gameplay Effect (Duration은 블루프린트에서 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Groggy")
	TSubclassOf<UGameplayEffect> GroggyStateEffect;

	/** 그로기 게이지를 리셋하는 Gameplay Effect (GroggyStateEffect의 Conditional Effect로 사용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Groggy")
	TSubclassOf<UGameplayEffect> GroggyResetEffect;

private:
	/** 태그 이벤트 델리게이트 핸들 */
	FDelegateHandle TagEventHandle;

	/** Shared.Status.Groggy 태그 변경 감지 콜백 */
	void OnGroggyTagChanged(const FGameplayTag Tag, int32 NewCount);

	/** 그로기 종료 처리 */
	void OnGroggyEnd();
};