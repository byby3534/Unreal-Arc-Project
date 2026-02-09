// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayTags/CMGameplayTags_Shared.h"

namespace CMGameplayTags
{
	#pragma region Shared Ability Tags
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_Death, "Shared.Ability.Death")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_Groggy, "Shared.Ability.Groggy")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_HitReact, "Shared.Ability.HitReact")
	#pragma endregion

	#pragma region Shared SetByCaller Tags
	UE_DEFINE_GAMEPLAY_TAG(Shared_SetByCaller_BaseDamage, "Shared.SetByCaller.BaseDamage")
	UE_DEFINE_GAMEPLAY_TAG(Shared_SetByCaller_GroggyDamage, "Shared.SetByCaller.GroggyDamage")
	UE_DEFINE_GAMEPLAY_TAG(Shared_SetByCaller_CounterAttackBonus, "Shared.SetByCaller.CounterAttackBonus")
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_SlowIntensity, "SetByCaller.SlowIntensity")
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Cooldown_Duration, "SetByCaller.Cooldown.Duration")
	#pragma endregion

	#pragma region Shared Status Tags
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_Dead, "Shared.Status.Dead")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_IsLockedOn, "Shared.Status.IsLockedOn")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_Groggy, "Shared.Status.Groggy")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact, "Shared.Status.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Front, "Shared.Status.HitReact.Front");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Left, "Shared.Status.HitReact.Left");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Back, "Shared.Status.HitReact.Back");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_HitReact_Right, "Shared.Status.HitReact.Right");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_SuperArmor, "Shared.Status.SuperArmor");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Status_Invincible, "Shared.Status.Invincible");

	#pragma endregion

	#pragma region Shared Event Tags
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_MeleeHit, "Shared.Event.MeleeHit")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_GroggyTriggered, "Shared.Event.GroggyTriggered")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_HitReact, "Shared.Event.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_Attack, "Shared.Event.Attack")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_Blink, "Shared.Event.Blink")
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_Death, "Shared.Event.Death")
	#pragma endregion

	#pragma region Shared Attack Tags
	UE_DEFINE_GAMEPLAY_TAG(Shared_Attack_Unparryable, "Shared.Attack.Unparryable");
	#pragma endregion
}