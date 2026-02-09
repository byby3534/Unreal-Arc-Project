// Copyright CrimsonMoon Team. All Rights Reserved.

#include "GameplayAbilitySystem/Abilities/Player/Blade/CMPlayerAbility_SkillCombo_Blade.h"

UCMPlayerAbility_SkillCombo_Blade::UCMPlayerAbility_SkillCombo_Blade()
{
	// Asset Tags는 블루프린트에서 설정
	// 예: Player.Ability.SkillCombo1, Player.Ability.SkillCombo2
}

void UCMPlayerAbility_SkillCombo_Blade::HandleComboComplete()
{
	// 콤보 리셋 없음 - 아무것도 안함
	// 다음번 스킬 사용 시 CurrentComboCount가 유지되어 다음 콤보부터 시작
}

void UCMPlayerAbility_SkillCombo_Blade::HandleComboCancelled()
{
	// [선택 1] 콤보 취소 시에도 리셋하지 않음 (현재 구현)
	// 다음번 스킬 사용 시 이어서 진행

	// [선택 2] 콤보 취소 시에는 리셋 (원하면 아래 주석 해제)
	// ResetComboCount();
}