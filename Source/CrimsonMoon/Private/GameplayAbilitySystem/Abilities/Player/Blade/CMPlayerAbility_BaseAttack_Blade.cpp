// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Player/Blade/CMPlayerAbility_BaseAttack_Blade.h"
#include "CMGameplayTags.h"
#include "TimerManager.h"

UCMPlayerAbility_BaseAttack_Blade::UCMPlayerAbility_BaseAttack_Blade()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Base_Attack);
	SetAssetTags(TagsToAdd);
}

void UCMPlayerAbility_BaseAttack_Blade::HandleComboComplete()
{
	// [서버 전용] 콤보 정상 완료 시 리셋 타이머 시작
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	// 이전 타이머가 있다면 클리어 (안전성)
	GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);

	// 리셋 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(ComboResetTimerHandle, this, &ThisClass::ResetCombo, ComboResetTime, false);
}

void UCMPlayerAbility_BaseAttack_Blade::HandleComboCancelled()
{
	// [서버 전용] 콤보 취소 시 즉시 리셋
	ResetCombo();
}

void UCMPlayerAbility_BaseAttack_Blade::ResetCombo()
{
	// [서버 전용] 콤보 리셋
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	// 타이머 클리어
	GetWorld()->GetTimerManager().ClearTimer(ComboResetTimerHandle);

	// 부모 클래스의 ResetComboCount 호출
	ResetComboCount();
}