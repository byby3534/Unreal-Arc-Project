// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Calculation/CMGECalculation_DamageTaken.h"
#include "CMGameplayTags.h"
#include "CrimsonMoon/DebugHelper.h"
#include "Structs/CMStructTypes.h"

static FCMDamageCapture& GetDamageCapture()
{
	static FCMDamageCapture DamageCapture;
	return DamageCapture;
}

UCMGECalculation_DamageTaken::UCMGECalculation_DamageTaken()
{
	RelevantAttributesToCapture.Add(GetDamageCapture().AttackPowerDef);
	RelevantAttributesToCapture.Add(GetDamageCapture().DefensePowerDef);
	RelevantAttributesToCapture.Add(GetDamageCapture().DamageTakenDef);
	RelevantAttributesToCapture.Add(GetDamageCapture().GroggyDamageTakenDef);
}

void UCMGECalculation_DamageTaken::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	const FGameplayEffectSpec& EffectSpec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvaluateParameters;

	EvaluateParameters.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	float SourceAttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageCapture().AttackPowerDef,
		EvaluateParameters,
		SourceAttackPower
		);

	// SetByCaller로 전달된 동적 값들 가져오기
	float BaseDamage = 0.f;
	float BaseGroggyDamage = 0.f;
	float CounterAttackBonus = 0.f; // 카운터 공격이 아니면 0
	int32 UsedLightAttackComboCount = 0;
	int32 UsedHeavyAttackComboCount = 0;

	for (const TPair<FGameplayTag, float>& TagMagnitude : EffectSpec.SetByCallerTagMagnitudes)
	{
		const float MagnitudeValue = TagMagnitude.Value;

		if (TagMagnitude.Key.MatchesTagExact(CMGameplayTags::Shared_SetByCaller_BaseDamage))
		{
			BaseDamage = MagnitudeValue;
		}
		if (TagMagnitude.Key.MatchesTagExact(CMGameplayTags::Player_SetByCaller_AttackType_Light))
		{
			UsedLightAttackComboCount = MagnitudeValue;
		}
		if (TagMagnitude.Key.MatchesTagExact(CMGameplayTags::Player_SetByCaller_AttackType_Heavy))
		{
			UsedHeavyAttackComboCount = MagnitudeValue;
		}
		if (TagMagnitude.Key.MatchesTagExact(CMGameplayTags::Shared_SetByCaller_CounterAttackBonus))
		{
			CounterAttackBonus = MagnitudeValue;
		}
		if (TagMagnitude.Key.MatchesTagExact(CMGameplayTags::Shared_SetByCaller_GroggyDamage))
		{
			BaseGroggyDamage = MagnitudeValue;
		}
	}

	float TargetDefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageCapture().DefensePowerDef,
		EvaluateParameters,
		TargetDefensePower
		);

	if (UsedLightAttackComboCount != 0)
	{
		const float DamageIncreasePercentLightAttack = (UsedLightAttackComboCount - 1) * 0.05f + 1.f;
		BaseDamage *= DamageIncreasePercentLightAttack;
	}

	if (UsedHeavyAttackComboCount != 0)
	{
		const float DamageIncreasePercentHeavyAttack = UsedHeavyAttackComboCount * 0.15f + 1.f;
		BaseDamage *= DamageIncreasePercentHeavyAttack;
	}

	// AttackPower와 DefensePower를 정규화된 값으로 해석
	// 예: AttackPower가 1.0이면 100% (기본), 1.5면 150% (1.5배)
	// 예: DefensePower가 0.6이면 60% 감소, 최종 데미지는 40%만 받음
	// 최대 방어율은 95%로 제한
	const float AttackMultiplier = SourceAttackPower;
	const float DefenseMultiplier = FMath::Clamp(TargetDefensePower, 0.f, 0.95f);
	float FinalDamageDone = BaseDamage * AttackMultiplier * (1.0f - DefenseMultiplier);

	// 카운터 공격 보너스 적용 (설정되어 있을 때만)
	if (CounterAttackBonus > 0.f)
	{
		FinalDamageDone *= CounterAttackBonus;
	}

	// 계산된 데미지를 출력(Output)으로 설정
	if (FinalDamageDone > 0.f)
	{
		const FGameplayModifierEvaluatedData ModifierEvaluatedData = FGameplayModifierEvaluatedData(
			GetDamageCapture().DamageTakenProperty,
			EGameplayModOp::Override,
			FinalDamageDone
			);

		// AttributeSet의 PostGameplayEffectExecute에서 이 DamageTaken 값을 읽어 CurrentHealth를 감소
		OutExecutionOutput.AddOutputModifier(ModifierEvaluatedData);
	}

	// 그로기 누적
	if (BaseGroggyDamage > 0.f)
	{
		const FGameplayModifierEvaluatedData GroggyModifier(
			GetDamageCapture().GroggyDamageTakenProperty,
			EGameplayModOp::Override,
			BaseGroggyDamage
			);

		OutExecutionOutput.AddOutputModifier(GroggyModifier);
	}
}