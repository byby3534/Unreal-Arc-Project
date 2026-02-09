// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayTags/CMGameplayTags_Input.h"

namespace CMGameplayTags
{
	#pragma region Input Tags
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look, "InputTag.Look")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Jump, "InputTag.Jump")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Toggleable, "InputTag.Toggleable")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Toggleable_LockOn, "InputTag.Toggleable.LockOn")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_BaseAttack, "InputTag.BaseAttack") // 삭제 예정
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Equip_Weapon, "InputTag.Equip.Weapon")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Unequip_Weapon, "InputTag.Unequip.Weapon")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_MustBeHeld, "InputTag.MustBeHeld")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_MustBeHeld_Block, "InputTag.MustBeHeld.Block")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_MustBeHeld_Skill_Blade_Issen, "InputTag.MustBeHeld.Skill.Blade.Q")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Block, "InputTag.Block")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Light, "InputTag.Attack.Light")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Heavy, "InputTag.Attack.Heavy")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Blade_Q, "InputTag.Skill.Blade.Q")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Blade_E, "InputTag.Skill.Blade.E")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Blade_R, "InputTag.Skill.Blade.R")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Blade_F, "InputTag.Skill.Blade.F")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Arcanist_Q, "InputTag.Skill.Arcanist.Q")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Arcanist_E, "InputTag.Skill.Arcanist.E")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Arcanist_R, "InputTag.Skill.Arcanist.R")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Arcanist_F, "InputTag.Skill.Arcanist.F")
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Action_Interact, "InputTag.Action.Interact")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Roll, "InputTag.Roll");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_QuickSlot_Utility, "InputTag.QuickSlot.Utility")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_QuickSlot_Potion, "InputTag.QuickSlot.Potion")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Action_UseItem, "InputTag.Action.UseItem")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Action_CycleItem, "InputTag.Action.CycleItem")
	#pragma endregion

	#pragma region UI Input Tags
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Back, "InputTag.UI.Back");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Inventory, "InputTag.UI.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_UI_Help, "InputTag.UI.Help");
	#pragma endregion
}