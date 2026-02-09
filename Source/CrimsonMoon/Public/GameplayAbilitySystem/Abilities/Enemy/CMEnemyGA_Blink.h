// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility.h"
#include "GameplayTags/CMGameplayTags_Shared.h"
#include "CMEnemyGA_Blink.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UCMEnemyGA_Blink : public UCMEnemyGameplayAbility
{
	GENERATED_BODY()

public:
	UCMEnemyGA_Blink();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 플레이어의 등 뒤로 이동할 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Blink")
	float DistanceBehindTargetToTeleport = 250.0f;

	/** AnimNotify로 전달받을 게임플레이 이벤트 태그 */
	UPROPERTY(EditDefaultsOnly, Category = "Blink")
	FGameplayTag TeleportEventTag = CMGameplayTags::Shared_Event_Blink;

	/** 순간이동 캐스팅 동작 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Blink")
	TObjectPtr<UAnimMontage> BlinkMontage;

private:
	UFUNCTION()
	void OnTeleportEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnMontageCancelled();
};