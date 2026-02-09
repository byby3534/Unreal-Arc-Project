// Fill out your copyright notice in the Description page of Project Settings.

#include "DataAssets/Input/CMDataAsset_InputConfig.h"
#include "Structs/CMStructTypes.h"

UInputAction* UCMDataAsset_InputConfig::FindNativeInputActionByTag(const FGameplayTag& InputTag) const
{
	for (const FCMInputActionConfig& InputActionConfig : NativeInputActions)
	{
		if (InputActionConfig.InputTag == InputTag && InputActionConfig.InputAction)
		{
			return InputActionConfig.InputAction;
		}
	}

	return nullptr;
}

UInputAction* UCMDataAsset_InputConfig::FindAbilityInputActionByTag(const FGameplayTag& InputTag) const
{
	for (const FCMInputActionConfig& InputActionConfig : AbilityInputActions)
	{
		if (InputActionConfig.InputTag == InputTag && InputActionConfig.InputAction)
		{
			return InputActionConfig.InputAction;
		}
	}

	return nullptr;
}
