// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayTags/CMGameplayTags_Currency.h"

namespace CMGameplayTags
{
	// 재화 변경 사유 태그
	UE_DEFINE_GAMEPLAY_TAG(Currency_Reason_Kill, "Currency.Reason.Kill");
	UE_DEFINE_GAMEPLAY_TAG(Currency_Reason_Upgrade, "Currency.Reason.Upgrade");
	UE_DEFINE_GAMEPLAY_TAG(Currency_Reason_LevelUp, "Currency.Reason.LevelUp");
	UE_DEFINE_GAMEPLAY_TAG(Currency_Reason_Purchase, "Currency.Reason.Purchase");
	UE_DEFINE_GAMEPLAY_TAG(Currency_Reason_Debug, "Currency.Reason.Debug");

	// 재화 관련 이벤트 태그 (확장용)
	UE_DEFINE_GAMEPLAY_TAG(Currency_Event_Gained, "Currency.Event.Gained");
	UE_DEFINE_GAMEPLAY_TAG(Currency_Event_Spent, "Currency.Event.Spent");

	// SetByCaller 태그 (GameplayEffect에서 동적 값 전달용)
	UE_DEFINE_GAMEPLAY_TAG(Currency_SetByCaller_Amount, "Currency.SetByCaller.Amount");
}
