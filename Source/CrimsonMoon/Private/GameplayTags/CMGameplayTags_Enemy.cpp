// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayTags/CMGameplayTags_Enemy.h"

namespace CMGameplayTags
{
	#pragma region Enemy Ability Tags
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Attack, "Enemy.Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Attack_Melee, "Enemy.Ability.Attack.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Attack_Ranged, "Enemy.Ability.Attack.Ranged");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_Phase2, "Enemy.Status.Phase2");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Status_Phase3, "Enemy.Status.Phase3");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Attack_Uppercut, "Enemy.Ability.Attack.Uppercut");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Attack_Slam, "Enemy.Ability.Attack.Slam");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_PhaseTransition, "Enemy.Ability.PhaseTransition");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Blink, "Enemy.Ability.Blink");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Attack_Ultimate, "Enemy.Ability.Attack.Ultimate");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_CursedKing_Weapon, "Enemy.CursedKing.Weapon");

	//임시
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Type_Common, "Enemy.Type.Common");
	#pragma endregion
}