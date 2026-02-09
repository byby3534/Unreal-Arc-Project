// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayTags/CMGameplayTags_MotionMatching.h"

namespace CMGameplayTags
{
	#pragma region Foley Tags
	UE_DEFINE_GAMEPLAY_TAG(Foley, "Foley");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event, "Foley.Event");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Handplant, "Foley.Event.Handplant");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Jump, "Foley.Event.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Land, "Foley.Event.Land");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Run, "Foley.Event.Run");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_RunBackwds, "Foley.Event.RunBackwds");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_RunStrafe, "Foley.Event.RunStrafe");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Scuff, "Foley.Event.Scuff");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_ScuffPivot, "Foley.Event.ScuffPivot");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_ScuffWall, "Foley.Event.ScuffWall");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Tumble, "Foley.Event.Tumble");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Walk, "Foley.Event.Walk");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_WalkBackwds, "Foley.Event.WalkBackwds");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Throw, "Foley.Event.Throw");
	UE_DEFINE_GAMEPLAY_TAG(Foley_Event_Immediate, "Foley.Event.Immediate");
	#pragma endregion

	#pragma region Ability Tags
	UE_DEFINE_GAMEPLAY_TAG(Abilities_ContextualAnim, "Abilities.ContextualAnim");
	UE_DEFINE_GAMEPLAY_TAG(Abilities_MoveTo, "Abilities.MoveTo");
	#pragma endregion

	#pragma region MotionMatching Tags
	UE_DEFINE_GAMEPLAY_TAG(MotionMatching, "MotionMatching");
	UE_DEFINE_GAMEPLAY_TAG(MotionMatching_Default, "MotionMatching.Default");
	UE_DEFINE_GAMEPLAY_TAG(MotionMatching_Idle, "MotionMatching.Idle");
	UE_DEFINE_GAMEPLAY_TAG(MotionMatching_Loops, "MotionMatching.Loops");
	UE_DEFINE_GAMEPLAY_TAG(MotionMatching_Pivots, "MotionMatching.Pivots");
	UE_DEFINE_GAMEPLAY_TAG(MotionMatching_Starts, "MotionMatching.Starts");
	UE_DEFINE_GAMEPLAY_TAG(MotionMatching_Stops, "MotionMatching.Stops");
	#pragma endregion
}