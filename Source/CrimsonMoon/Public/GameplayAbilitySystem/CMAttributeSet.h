// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CMAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * @class UCMAttributeSet
 * @brief UCMAttributeSet 클래스는 Crimson Moon 프로젝트의 캐릭터 속성(Attribute)을 정의하고 관리하는 역할을 수행합니다.
 *
 * 이 클래스는 Unreal Engine의 UAttributeSet를 상속받아 구현되었으며, 게임 플레이 능력 시스템(GAS: Gameplay Ability System)과 연계되어
 * 캐릭터의 여러 속성(health, mana, strength 등)을 효율적으로 관리하거나 업데이트하는 데 사용됩니다.
 *
 * UCMAttributeSet은 ACMCharacterBase 클래스와 같은 캐릭터 클래스에서 주로 사용되며, 캐릭터의 상태와 능력에 따라
 * 속성값이 동적으로 변경될 수 있습니다.
 *
 * [가스 시스템 연계]
 * - GAS를 통해 적용되는 능력(Abilities)을 활성화하거나 종료할 때 속성 값을 자동으로 계산 및 업데이트합니다.
 * - 예: 캐릭터의 체력이 감소하거나 스킬에 의해 스탯이 변화될 경우, 속성을 변경하고 이를 다른 관련 시스템에 알립니다.
 *
 */

class IPawnUIInterface;

UCLASS()
class CRIMSONMOON_API UCMAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCMAttributeSet();

	#pragma region Macro Function

	ATTRIBUTE_ACCESSORS(UCMAttributeSet, CurrentHealth);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, CurrentMana);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, MaxMana);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, MaxStamina);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, CurrentStamina);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, DamageTaken);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, AttackPower);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, DefensePower);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, CurrentGroggy);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, MaxGroggy);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, GroggyDamageTaken);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, AttackSpeed);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, MoveSpeed);

	// 재화 Attribute (float 기반이지만 정수처럼 운용)
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, CurrentCurrency);
	ATTRIBUTE_ACCESSORS(UCMAttributeSet, MaxCurrency);

	#pragma endregion

	#pragma region Attribute Overrides

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	#pragma endregion

	#pragma region Player State Attributes

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player State", ReplicatedUsing = OnRep_CurrentHealth)
	FGameplayAttributeData CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player State", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player State", ReplicatedUsing = OnRep_CurrentMana)
	FGameplayAttributeData CurrentMana;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player State", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player State", ReplicatedUsing = OnRep_CurrentStamina)
	FGameplayAttributeData CurrentStamina;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player State", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player Combat", ReplicatedUsing = OnRep_BaseDamage)
	FGameplayAttributeData BaseDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player Combat", ReplicatedUsing = OnRep_DamageTaken)
	FGameplayAttributeData DamageTaken;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player Combat", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Player Combat", ReplicatedUsing = OnRep_DefensePower)
	FGameplayAttributeData DefensePower;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Groggy", ReplicatedUsing = OnRep_CurrentGroggy)
	FGameplayAttributeData CurrentGroggy;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Groggy", ReplicatedUsing = OnRep_MaxGroggy)
	FGameplayAttributeData MaxGroggy;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Groggy", ReplicatedUsing = OnRep_GroggyDamageTaken)
	FGameplayAttributeData GroggyDamageTaken;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Combat", ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Movement", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;

	#pragma endregion

	#pragma region Currency Attributes

	/**
	 * 현재 보유 재화 (Soul/Rune)
	 * float 기반 GAS Attribute이지만 정수처럼 운용됨
	 * 모든 값 변경 시 FMath::RoundToInt를 적용하여 소수점을 방지
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Currency", ReplicatedUsing = OnRep_CurrentCurrency)
	FGameplayAttributeData CurrentCurrency;

	/**
	 * 최대 보유 가능 재화 (선택적 상한선, 0이면 무제한)
	 * 소울라이크 게임에서는 보통 무제한이지만, 확장성을 위해 추가
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attribute|Currency", ReplicatedUsing = OnRep_MaxCurrency)
	FGameplayAttributeData MaxCurrency;

	#pragma endregion

	#pragma region Replication Functions
	UFUNCTION()
	void OnRep_CurrentHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CurrentMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CurrentStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DamageTaken(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DefensePower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CurrentGroggy(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxGroggy(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_GroggyDamageTaken(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CurrentCurrency(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxCurrency(const FGameplayAttributeData& OldValue);

	#pragma endregion

private:
	/**
	 * 블록 또는 패링 시도를 처리합니다.
	 * @param TargetASC 방어자의 어빌리티 시스템 컴포넌트
	 * @param SourceASC 공격자의 어빌리티 시스템 컴포넌트 (null 가능)
	 * @param Attacker 공격자 액터
	 * @param Defender 방어자 액터
	 * @param bOutWasBlockAttempted [out] 블록을 시도했는지 여부 (방향 무관)
	 * @param bOutBlockSucceeded [out] 블록 성공 여부 (방향 체크 통과, 퍼펙트 패링 제외)
	 * @return 퍼펙트 패링 성공 시 true (데미지 완전 무효화), 일반 블록 또는 실패 시 false
	 */
	bool TryBlockOrParry(UAbilitySystemComponent* TargetASC, UAbilitySystemComponent* SourceASC, AActor* Attacker, AActor* Defender, bool& bOutWasBlockAttempted, bool& bOutBlockSucceeded) const;

	/**
	 * @brief 그로기 데미지를 누적 처리하고 조건 충족 시 그로기 상태를 트리거합니다.
	 *
	 * Invincible / SuperArmor 태그가 없고 이미 그로기 상태가 아닐 경우,
	 * GroggyDamageTaken을 누적하여 MaxGroggy 초과 시 그로기 이벤트를 발생시킵니다.
	 *
	 * @param Data GameplayEffect 적용 시 전달되는 콜백 데이터
	 */
	void HandleGroggyDamage(const FGameplayEffectModCallbackData& Data);

	/**
	 * @brief 데미지를 처리하고 조건에 따라 히트 리액션을 트리거합니다.
	 *
	 * 이 함수는 DamageTaken 속성이 변경될 때 호출되며, 다음 순서로 처리됩니다:
	 * 1. 무적(Invincible) 상태 체크 - 무적 시 데미지 무효화 및 조기 반환
	 * 2. 블록/패링 판정 - TryBlockOrParry()를 통해 방어 성공 여부 확인
	 * 3. 데미지 적용 - 퍼펙트 패링 실패 시에만 체력 감소
	 * 4. 사망 판정 - 체력이 0 이하일 경우 Dead 태그 부여
	 * 5. 히트 리액션 - 패링 실패 및 면역 태그(Groggy, SuperArmor, Invincible)가 없을 경우 히트 리액션 이벤트 발생
	 *
	 * @param Data GameplayEffect 적용 시 전달되는 콜백 데이터
	 */
	void HandleDamageAndTriggerHitReact(const FGameplayEffectModCallbackData& Data);
};