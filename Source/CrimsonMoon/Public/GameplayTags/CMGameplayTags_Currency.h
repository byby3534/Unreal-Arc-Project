// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * 재화 시스템 관련 GameplayTag 정의
 *
 * 사용법:
 * - currency.reason.kill: 몬스터 처치로 인한 재화 획득
 * - currency.reason.upgrade: 장비/스킬 강화로 인한 재화 소비
 * - currency.reason.levelup: 레벨업으로 인한 재화 소비
 * - currency.reason.purchase: 상점 구매로 인한 재화 소비
 * - currency.reason.debug: 디버그/치트 명령으로 인한 재화 변경
 *
 * 확장 포인트:
 * - 다중 재화 지원 시 currency.type.souls, currency.type.gold 등 추가
 */
namespace CMGameplayTags
{
	// 재화 변경 사유 태그
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency_Reason_Kill);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency_Reason_Upgrade);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency_Reason_LevelUp);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency_Reason_Purchase);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency_Reason_Debug);

	// 재화 관련 이벤트 태그 (확장용)
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency_Event_Gained);
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency_Event_Spent);

	// SetByCaller 태그 (GameplayEffect에서 동적 값 전달용)
	CRIMSONMOON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency_SetByCaller_Amount);
}
