// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CMGECalculation_DamageTaken.generated.h"

/**
 * @class UCMGECalculation_DamageTaken
 * @brief 피해량 계산을 처리하는 클래스입니다.
 *
 * 이 클래스는 UGameplayEffectExecutionCalculation을 상속받아, 피해량 관련 로직 및 계산식을
 * 게임플레이 시스템에 구현하기 위한 목적으로 사용됩니다.
 *
 * UCMGECalculation_DamageTaken은 게임 내에서 특정 효과나 능력치 변경으로 인해
 * 발생하는 피해량 데이터를 계산하고 처리하는 데 사용됩니다.
 *
 *
 * - 이 클래스는 피해량 계산의 규칙이나 공식을 구현할 수 있는 확장 가능성이 있습니다.
 * - 일반적으로 게임플레이 효과(UGameplayEffect)와 결합하여 사용됩니다.
 * - 필요한 경우 오버라이딩을 통해 사용자 정의 계산 로직을 추가할 수 있습니다.
 */
UCLASS()
class CRIMSONMOON_API UCMGECalculation_DamageTaken : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	/**
	 * @brief 데미지 계산에 필요한 속성 캡처를 초기화
	 */
	UCMGECalculation_DamageTaken();

	/**
	 * @brief 공격자·피격자 능력치와 콤보 정보를 기반으로 최종 피해를 계산합니다.
	 *
	 * - 공격력, 방어력, BaseDamage, 콤보 횟수 정보를 가져옴  
	 * - 콤보에 따른 피해 보정 적용  
	 * - 최종 피해(FinalDamage) 계산 후 OutExecutionOutput에 반영
	 * @param ExecutionParams 공격자·피격자 속성과 효과 정보를 포함하는 입력 파라미터입니다.
	 * @param OutExecutionOutput 최종 계산된 피해값이 기록되는 출력 파라미터입니다.
	 */
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};