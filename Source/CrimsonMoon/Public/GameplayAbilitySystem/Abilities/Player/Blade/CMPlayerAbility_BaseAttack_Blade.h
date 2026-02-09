// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CMGameplayTags.h"
#include "GameplayAbilitySystem/Abilities/Player/Blade/CMPlayerAbility_ComboAttack_Base.h"
#include "CMPlayerAbility_BaseAttack_Blade.generated.h"

/**
 * @class UCMPlayerAbility_BaseAttack_Blade
 * @brief '블레이드' 캐릭터의 기본 공격 콤보를 처리하는 게임플레이 어빌리티입니다.
 *
 * [특징]
 * - ComboAttack_Base를 상속하여 콤보 시스템 재사용
 * - 콤보 리셋 타이머 기능 추가 (일정 시간 후 콤보 초기화)
 *
 * [리슨 서버 고려]
 * - 콤보 리셋 타이머는 서버에서만 관리
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAbility_BaseAttack_Blade : public UCMPlayerAbility_ComboAttack_Base
{
	GENERATED_BODY()

public:
	UCMPlayerAbility_BaseAttack_Blade();

protected:
	/** [오버라이드] 콤보 정상 완료 시 리셋 타이머 시작 */
	virtual void HandleComboComplete() override;

	/** [오버라이드] 콤보 취소 시 즉시 리셋 */
	virtual void HandleComboCancelled() override;

private:
	/** 콤보가 리셋되기까지의 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (AllowPrivateAccess = "true"))
	float ComboResetTime = 1.0f;

	/** [서버 전용] 콤보 리셋 타이머 핸들 */
	FTimerHandle ComboResetTimerHandle;

	/** [서버 전용] 콤보 리셋 */
	void ResetCombo();
};