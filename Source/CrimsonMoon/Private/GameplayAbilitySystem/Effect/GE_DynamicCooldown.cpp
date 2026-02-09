// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Effect/GE_DynamicCooldown.h"

#include "GameplayTags/CMGameplayTags_Shared.h"

UGE_DynamicCooldown::UGE_DynamicCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat SetByCallerInfo;
	SetByCallerInfo.DataTag = CMGameplayTags::SetByCaller_Cooldown_Duration; 

	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCallerInfo);
}