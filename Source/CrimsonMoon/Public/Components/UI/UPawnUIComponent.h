// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnExtensionComponentBase.h"
#include "AbilitySystemComponent.h"
#include "Structs/CMStructTypes.h"
#include "UPawnUIComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPercentChangedDelegate, float, NewPercent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityIconSlotDelegate, FAbilityIconData, IconData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCooldownStartedDelegate, FGameplayTag, SlotTag, float, Duration);

// 재화 변경 델리게이트 (UI 갱신용)
// @param NewAmount 변경 후 재화량
// @param DeltaAmount 변경량 (양수: 획득, 음수: 소비)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyUIChangedDelegate, int32, NewAmount, int32, DeltaAmount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CRIMSONMOON_API UPawnUIComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	UPawnUIComponent();

	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentManaChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentStaminaChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityIconSlotDelegate OnAbilityIconSlot;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityCooldownStartedDelegate OnAbilityCooldownStarted;

	/**
	 * 재화가 변경되었을 때 브로드캐스트되는 델리게이트
	 * UI 위젯에서 바인딩하여 재화 표시를 갱신할 수 있습니다.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnCurrencyUIChangedDelegate OnCurrencyChanged;

	// ASC 델리게이트 구독 초기화 (서버/클라이언트 모두 자동 작동)
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeWithASC(UAbilitySystemComponent* ASC);

	// 초기값 브로드캐스트
	UFUNCTION(BlueprintCallable, Category = "UI")
	void BroadcastInitStatusValues();

	// 서버에서 호출: Ability 아이콘 추가
	void AddAbilityIcon(const FCMPlayerAbilitySet& AbilitySet);

	// 서버에서 호출: Ability 아이콘 제거
	void RemoveAbilityIcon(FGameplayTag SlotTag);

	// 초기 아이콘들 브로드캐스트 (UI 초기화 시 호출)
	UFUNCTION(BlueprintCallable, Category = "UI")
	void BroadcastInitAbilityIcons();

	// Ability 쿨타임 시작 (InputTag로 SlotTag 찾아서 브로드캐스트)
	void StartAbilityCooldown(const FGameplayTag& InputTag, float Duration);

protected:
	virtual void BeginDestroy() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 리플리케이트되는 어빌리티 아이콘 배열
	UPROPERTY(ReplicatedUsing = OnRep_AbilityIcons)
	TArray<FAbilityIconData> AbilityIcons;

	UFUNCTION()
	void OnRep_AbilityIcons();

	// ASC 델리게이트 핸들 (언바인딩용)
	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle MaxHealthChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
	FDelegateHandle MaxManaChangedDelegateHandle;
	FDelegateHandle StaminaChangedDelegateHandle;
	FDelegateHandle MaxStaminaChangedDelegateHandle;
	FDelegateHandle CurrencyChangedDelegateHandle;

	// ASC 참조
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	// 속성 변경 콜백
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data);
	void OnManaAttributeChanged(const FOnAttributeChangeData& Data);
	void OnMaxManaAttributeChanged(const FOnAttributeChangeData& Data);
	void OnStaminaAttributeChanged(const FOnAttributeChangeData& Data);
	void OnMaxStaminaAttributeChanged(const FOnAttributeChangeData& Data);
	void OnCurrencyAttributeChanged(const FOnAttributeChangeData& Data);

	// 재화 변경량 추적용 (델타 계산)
	int32 CachedCurrencyAmount = 0;
};