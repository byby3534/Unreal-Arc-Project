// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerAbility_Block.h"
#include "CMPlayerAbility_Block_Arcanist.generated.h"

/**
 * 아케이니스트 전용 막기 어빌리티
 * 활성화 시 전방에 구체 배리어를 소환합니다.
 * 종료 시 배리어를 제거합니다.
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_Block_Arcanist : public UCMPlayerAbility_Block
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	// 소환할 배리어 액터 클래스 (블루프린트에서 설정)
	UPROPERTY(EditDefaultsOnly, Category = "Arcanist|Barrier")
	TSubclassOf<AActor> BarrierActorClass;

	// 생성된 배리어 인스턴스
	UPROPERTY()
	TObjectPtr<AActor> SpawnedBarrier;

	// 배리어 소환 위치 오프셋 (캐릭터 기준)
	UPROPERTY(EditDefaultsOnly, Category = "Arcanist|Barrier")
	FVector BarrierSpawnOffset = FVector(100.f, 0.f, 0.f);

private:
	// 퍼펙트 패링 성공 시 호출 (카운터 어택 비활성화를 위해 오버라이드)
	virtual void OnPerfectParrySuccess(FGameplayEventData Payload) override;

	// 몽타주 종료 시 어빌리티 종료 (아케이니스트는 한 번 누르면 몽타주 재생 후 종료)
	virtual void OnMontageEnded() override;
};