// Copyright CrimsonMoon Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Abilities/Player/Blade/CMPlayerAbility_ComboAttack_Base.h"
#include "CMPlayerAbility_SkillCombo_Blade.generated.h"

/**
 * @class UCMPlayerAbility_SkillCombo_Blade
 * @brief '블레이드' 캐릭터의 스킬 콤보를 처리하는 게임플레이 어빌리티입니다.
 *
 * [특징]
 * - ComboAttack_Base를 상속하여 콤보 시스템 재사용
 * - 콤보 리셋 타이머 없음 (연속 타격이 계속 이어짐)
 * - Trail 이펙트 등은 GameplayCue로 처리 (몽타주에 설정)
 * 
 * [리슨 서버]
 * - 콤보는 리셋되지 않고 계속 이어짐
 * - 스킬 사용 후 다시 사용하면 다음 콤보부터 시작
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_SkillCombo_Blade : public UCMPlayerAbility_ComboAttack_Base
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_SkillCombo_Blade();

protected:
	/** [오버라이드] 콤보 정상 완료 시 아무것도 안함 (리셋 없음) */
	virtual void HandleComboComplete() override;

	/** [오버라이드] 콤보 취소 시에도 리셋하지 않음 (선택적으로 변경 가능) */
	virtual void HandleComboCancelled() override;
};