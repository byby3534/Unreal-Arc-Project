// Fill out your copyright notice in the Description page of Project Settings.

#include "Structs/CMStructTypes.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"

bool FCMInputActionConfig::IsValid() const
{
	return InputTag.IsValid() && InputAction;
}

bool FCMPlayerAbilitySet::IsValid() const
{
	return InputTag.IsValid() && AbilityToGrant;
}
