// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "CMPlayerAbility_EquipConsumable.generated.h"

class UCMDataAsset_ConsumableData;
class UCMItemInstance;

/**
 * 소모품을 손에 드는 어빌리티
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_EquipConsumable : public UCMPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_EquipConsumable();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:

	// [장착형] 장착 몽타주 노티파이 수신
	UFUNCTION()
	void OnEquipEventReceived(FGameplayEventData Payload);

	// [즉발형] 섭취 몽타주 노티파이 수신
	UFUNCTION()
	void OnConsumeEventReceived(FGameplayEventData Payload);

	// 로직 분리기
	void HandleImmediateItem();      // 즉시 사용 (물약 등)
	void HandleEquippableItem(UCMItemInstance* InItemInstance);     // 장착 대기 (수류탄, 설치물 등)

protected:
	
	// 데이터 에셋 캐싱
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	TObjectPtr<const UCMDataAsset_ConsumableData> ConsumableData;
    
	// 장착 시 재생할 기본 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DefaultEquipMontage;
    
	// 이펙트/메쉬 스폰될 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "Socket")
	FName HandSocketName = FName("Hand_Item");

	// 즉발형 섭취 타이밍 감지 태그
	UPROPERTY(EditDefaultsOnly, Category = "Immediate")
	FGameplayTag ConsumeEventTag;

	// 장착형 장착 타이밍 감지 태그
	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	FGameplayTag EquipEventTag;

private:
	// 장착형 아이템을 위해 임시로 부여한 어빌리티 핸들
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
};
