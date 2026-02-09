// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayTags/CMGameplayTags_Player.h"

namespace CMGameplayTags
{
	#pragma region Player Ability Tags
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability, "Player.Ability")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Base_Attack, "Player.Ability.Base.Attack")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_BaseAttack, "Player.Ability.BaseAttack")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_TargetLock, "Player.Ability.TargetLock")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_EquipWeapon, "Player.Ability.EquipWeapon")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_EquipConsumable, "Player.Ability.EquipConsumable")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_UnEquipWeapon, "Player.Ability.UnEquipWeapon")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_UnEquipConsumable, "Player.Ability.UnEquipConsumable")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Block, "Player.Ability.Block")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Rolling, "Player.Ability.Rolling")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Skill_Blade_Q, "Player.Ability.Skill.Blade.Q")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Skill_Blade_E, "Player.Ability.Skill.Blade.E")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Skill_Blade_R, "Player.Ability.Skill.Blade.R")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Skill_Blade_F, "Player.Ability.Skill.Blade.F")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Skill_Arcanist_Q, "Player.Ability.Skill.Arcanist.Q")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Skill_Arcanist_E, "Player.Ability.Skill.Arcanist.E")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Skill_Arcanist_R, "Player.Ability.Skill.Arcanist.R")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Skill_Arcanist_F, "Player.Ability.Skill.Arcanist.F")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Item, "Player.Ability.Item");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Item_Throw, "Player.Ability.Item.Throw");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Item_Place, "Player.Ability.Item.Place");

	UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Judgement_Sword, "Player.Weapon.Judgement.Sword")
	UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Arcanist_TestWeapon, "Player.Weapon.Arcanist.TestWeapon")

	UE_DEFINE_GAMEPLAY_TAG(Player_Weapon_Judgement_LegendSword, "Player.Weapon.Judgement.LegendSword")
	#pragma endregion

	#pragma region Player SendGameplayEvent Tags
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_HitPause, "Player.Event.HitPause")
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_EquipWeapon, "Player.Event.EquipWeapon");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_EquipConsume, "Player.Event.EquipConsume");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_UnequipWeapon, "Player.Event.UnequipWeapon");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_ParryWindowStart, "Player.Event.ParryWindowStart");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_FireProjectile, "Player.Event.FireProjectile");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_PerfectParrySuccess, "Player.Event.PerfectParrySuccess");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SuccessfulBlock, "Player.Event.SuccessfulBlock");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SoulSiphon_FirstDamage, "Player.Event.SoulSiphon.FirstDamage");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SoulSiphon_SecondDamage, "Player.Event.SoulSiphon.SecondDamage");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Advent_ThunderStrike, "Player.Event.Advent.ThunderStrike");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_SoulLaser_Start, "Player.Event.SoulLaser.Start");
	#pragma endregion

	#pragma region Player Status Tags
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Blocking, "Player.Status.Blocking")
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Equipping, "Player.Status.Equipping")
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_PerfectParryWindow, "Player.Status.PerfectParryWindow")
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_CanCounterAttack, "Player.Status.CanCounterAttack")
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Charging, "Player.Status.Charging")
	UE_DEFINE_GAMEPLAY_TAG(Player_Status_Rolling, "Player.Status.Rolling")
	#pragma endregion

	#pragma region Player SetByCaller Tags
	UE_DEFINE_GAMEPLAY_TAG(Player_SetByCaller_AttackType_Light, "Player.SetByCaller.AttackType.Light")
	UE_DEFINE_GAMEPLAY_TAG(Player_SetByCaller_AttackType_Heavy, "Player.SetByCaller.AttackType.Heavy")
	#pragma endregion

	#pragma region Player Skill Slot
	UE_DEFINE_GAMEPLAY_TAG(Player_Skill_Slot_Q, "Player.Skill.Slot.Q")
	UE_DEFINE_GAMEPLAY_TAG(Player_Skill_Slot_E, "Player.Skill.Slot.E")
	UE_DEFINE_GAMEPLAY_TAG(Player_Skill_Slot_R, "Player.Skill.Slot.R")
	UE_DEFINE_GAMEPLAY_TAG(Player_Skill_Slot_F, "Player.Skill.Slot.F")
	#pragma endregion

	#pragma region Player Cooldown Tags
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_Arcanist_Skill_Q, "Cooldown.Arcanist.Skill.Q")
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_Arcanist_Skill_E, "Cooldown.Arcanist.Skill.E")
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_Arcanist_Skill_R, "Cooldown.Arcanist.Skill.R")
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_Arcanist_Skill_F, "Cooldown.Arcanist.Skill.F")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Arcanist_Skill_BaseAttack, "Cooldown.Arcanist.Skill.BaseAttack")
	#pragma endregion

	#pragma region Player State Tags
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Buff_InfiniteStamina, "Player.State.Buff.InfiniteStamina")
	UE_DEFINE_GAMEPLAY_TAG(Player_State_IsSprint, "Player.State.IsSprint")
	#pragma endregion
}