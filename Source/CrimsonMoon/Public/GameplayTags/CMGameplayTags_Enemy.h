// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace CMGameplayTags
{
	#pragma region Enemy Ability Tags
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Attack);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Attack_Melee);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Attack_Ranged);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Status_Phase2);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Status_Phase3);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Attack_Uppercut);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Attack_Slam);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_PhaseTransition);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Blink);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Attack_Ultimate);

	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_CursedKing_Weapon);

	// 임시 적
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Type_Common);
	#pragma endregion
}