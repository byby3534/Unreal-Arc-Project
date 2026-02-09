// Fill out your copyright notice in the Description page of Project Settings.

#include "DataAssets/Startup/CMDataAsset_StartupDataBase.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"

void UCMDataAsset_StartupDataBase::GiveToAbilitySystemComponent(UCMAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	GrantAbilities(ActivateOnGivenAbilities, InASCToGive, ApplyLevel);
	GrantAbilities(ReactiveAbilities, InASCToGive, ApplyLevel);

	if (!StartUpGameplayEffects.IsEmpty())
	{
		for (const TSubclassOf<UGameplayEffect>& EffectsClass : StartUpGameplayEffects)
		{
			if (!EffectsClass)
			{
				continue;
			}
			const UGameplayEffect* EffectClassDefaultObject = EffectsClass->GetDefaultObject<UGameplayEffect>();
			InASCToGive->ApplyGameplayEffectToSelf(EffectClassDefaultObject, ApplyLevel, InASCToGive->MakeEffectContext());
		}
	}
}

void UCMDataAsset_StartupDataBase::GrantAbilities(
	const TArray<TSubclassOf<UCMGameplayAbility>>& InAbilitiesToGive,
	UCMAbilitySystemComponent* InASCToGive,
	int32 ApplyLevel)
{
	if (InAbilitiesToGive.IsEmpty())
	{
		return;
	}

	for (const TSubclassOf<UCMGameplayAbility>& Ability : InAbilitiesToGive)
	{
		if (!Ability)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;

		InASCToGive->GiveAbility(AbilitySpec);
	}
}