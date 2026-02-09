// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "CMAbilitySystemComponent.generated.h"

/**
 * 재화 변경 시 브로드캐스트되는 델리게이트
 * @param NewAmount 변경 후 재화 양
 * @param DeltaAmount 변경량 (양수: 획득, 음수: 소비)
 * @param ReasonTag 변경 사유 태그
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCurrencyChangedDelegate, int32, NewAmount, int32, DeltaAmount, FGameplayTag, ReasonTag);

/**
 * @class UCMAbilitySystemComponent
 * @brief Crimson Moon 게임의 확장된 Ability System Component 클래스.
 *
 * `UCMAbilitySystemComponent`는 게임플레이에서 능력 시스템을 관리하기 위한 클래스입니다.
 * 기본 UAbilitySystemComponent를 상속받아 사용자 정의 기능을 확장합니다.
 *
 * [재화 시스템]
 * - AddCurrency/SpendCurrency: 서버 권한 기반 재화 증감 API
 * - CanAfford: 비용 지불 가능 여부 확인
 * - 클라이언트 호출 시 Server RPC를 통해 서버에서 처리
 */
UCLASS()
class CRIMSONMOON_API UCMAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void ServerAddLooseGameplayTag(FGameplayTag TagToAdd);

	void OnAbilityInputPressed(const FGameplayTag& InputTag);
	void OnAbilityInputReleased(const FGameplayTag& InputTag);

	#pragma region Currency System

	/* 재화 변경 시 브로드캐스트되는 델리게이트, UI 바인딩 또는 게임플레이 로직에서 사용 */
	UPROPERTY(BlueprintAssignable, Category = "Currency")
	FOnCurrencyChangedDelegate OnCurrencyChanged;

	/**
	 * 재화를 추가
	 * 서버에서만 실제 값이 변경되며, 클라이언트 호출 시 Server RPC를 통해 서버로 전달
	 *
	 * @param Amount 추가할 재화량 (음수는 무시됨)
	 * @param ReasonTag 변경 사유 태그 (예: Currency.Reason.Kill)
	 * @param Source 재화 소스 액터 (옵션, 로깅/디버그용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency", meta = (GameplayTagFilter = "Currency.Reason"))
	void AddCurrency(int32 Amount, FGameplayTag ReasonTag, UObject* Source = nullptr);

	/**
	 * 재화를 소비
	 * 서버에서만 실제 값이 변경되며, 클라이언트 호출 시 Server RPC를 통해 서버로 전달
	 * CanAfford()로 미리 확인 후 호출하는 것을 권장
	 *
	 * @param Cost 소비할 재화량 (음수는 무시됨)
	 * @param ReasonTag 변경 사유 태그 (예: Currency.Reason.Upgrade)
	 * @param bSuccess [out] 소비 성공 여부 (잔액 부족 시 false)
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency", meta = (GameplayTagFilter = "Currency.Reason"))
	void SpendCurrency(int32 Cost, FGameplayTag ReasonTag, bool& bSuccess);

	/**
	 * 특정 비용을 지불할 수 있는지 확인
	 * 클라이언트에서도 호출 가능 (Replicated Attribute 사용)
	 *
	 * @param Cost 확인할 비용
	 * @return 지불 가능 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool CanAfford(int32 Cost) const;

	/**
	 * 현재 보유 재화량을 반환
	 * 클라이언트에서도 호출 가능 (Replicated Attribute 사용)
	 *
	 * @return 현재 재화량 (정수)
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCurrentCurrencyAmount() const;

protected:
	/* Server RPC: 재화 추가, 클라이언트에서 AddCurrency 호출 시 내부적으로 사용 */
	UFUNCTION(Server, Reliable)
	void ServerAddCurrency(int32 Amount, FGameplayTag ReasonTag);

	/* Server RPC: 재화 소비, 클라이언트에서 SpendCurrency 호출 시 내부적으로 사용 */
	UFUNCTION(Server, Reliable)
	void ServerSpendCurrency(int32 Cost, FGameplayTag ReasonTag);

private:
	/**
	 * 서버에서 실제 재화 변경을 수행
	 * AddCurrency, SpendCurrency의 내부 구현
	 *
	 * @param DeltaAmount 변경량 (양수: 획득, 음수: 소비)
	 * @param ReasonTag 변경 사유 태그
	 * @return 변경 성공 여부
	 */
	bool Internal_ModifyCurrency(int32 DeltaAmount, FGameplayTag ReasonTag);

	/**
	 * GameplayEffect를 사용하여 재화를 변경
	 * SetByCaller로 Amount를 전달하여 CurrentCurrency Attribute 수정
	 *
	 * @param DeltaAmount 변경량 (양수: 획득, 음수: 소비)
	 * @return GE 적용 성공 여부
	 */
	bool ApplyCurrencyModifyEffect(int32 DeltaAmount);

	/**
	 * 재화 로그를 출력 (디버그 옵션)
	 */
	void LogCurrencyChange(int32 OldAmount, int32 NewAmount, int32 DeltaAmount, const FGameplayTag& ReasonTag, bool bIsGain) const;

	/** 재화 수정용 GameplayEffect 클래스 (런타임 생성) */
	UPROPERTY()
	TSubclassOf<UGameplayEffect> CurrencyModifyEffectClass;

	/** GE 클래스 초기화 */
	void EnsureCurrencyEffectClassInitialized();

	#pragma endregion
};