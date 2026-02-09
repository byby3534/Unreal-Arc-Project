// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace CMGameplayTags
{
	#pragma region Shared Ability Tags
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Ability_Death);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Ability_Groggy);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Ability_HitReact);
	#pragma endregion

	#pragma region Shared SetByCaller Tags
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_SetByCaller_BaseDamage);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_SetByCaller_GroggyDamage);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_SetByCaller_CounterAttackBonus);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_SlowIntensity);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Cooldown_Duration);
	#pragma endregion

	#pragma region Shared Status Tags
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_Dead);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_IsLockedOn);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_Groggy);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact_Front);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact_Left);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact_Back);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_HitReact_Right);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_SuperArmor);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Status_Invincible);
	#pragma endregion

	#pragma region Shared Event Tags
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_MeleeHit);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_GroggyTriggered);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_HitReact);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_Attack);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_Blink);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Event_Death);
	#pragma endregion

	#pragma region Shared Attack Tags
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Shared_Attack_Unparryable);
	#pragma endregion

}