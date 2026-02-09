// Fill out your copyright notice in the Description page of Project Settings.

#include "DataAssets/Startup/CMDataAsset_EnemyStartupData.h"
#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"

void UCMDataAsset_EnemyStartupData::GiveToAbilitySystemComponent(UCMAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	Super::GiveToAbilitySystemComponent(InASCToGive, ApplyLevel);

	if (!EnemyGameplayAbility.IsEmpty())
	{
		for (const TSubclassOf<UCMGameplayAbility>& AbilityClass : EnemyGameplayAbility)
		{
			if (!AbilityClass)
			{
				continue;
			}
			FGameplayAbilitySpec AbilitySpec(AbilityClass);
			AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
			AbilitySpec.Level = ApplyLevel;

			InASCToGive->GiveAbility(AbilitySpec);
		}
	}
}