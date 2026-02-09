// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * @brief 공격 판정에 사용하는 손/무기의 종류를 정의하는 열거형 클래스입니다.
 *
 * 이 열거형은 공격 시 판별되는 손/무기의 타입을 나타냅니다.
 * 각각의 요소는 다음과 같습니다:
 * - CurrentEquippedWeapon: 무기를 장착했을때 호축
 * - LeftHand: 왼손 콜리전 활성화
 * - RightHand: 오른손 콜리전 활성화
 *
 */
UENUM(BlueprintType)
enum class EToggleDamageType : uint8
{
	CurrentEquippedWeapon,
	LeftHand,
	RightHand
};

UENUM()
enum class ECMValidType : uint8
{
	Valid,
	Invalid,
};

UENUM()
enum class ETeamType : uint8
{
	None = 0,

	Player,
	Enemy,
	NPC,

	END = 99
};

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	Instant UMETA(DisplayName = "Instant Press"),
	Hold UMETA(DisplayName = "Hold to Interact"),
};

UENUM(BlueprintType)
enum class EUILayerType : uint8
{
	None UMETA(DisplayName = "None"),
	HUD UMETA(DisplayName = "HUD"),
	GameMenu UMETA(DisplayName = "Game Menu"),
	MainMenu UMETA(DisplayName = "Main Menu"),
	Popup UMETA(DisplayName = "Popup")
};

UENUM(BlueprintType)
enum class EUIInputMode : uint8
{
	GameOnly UMETA(DisplayName = "GameOnly"),
	GameAndUI UMETA(DisplayName = "GameAndUI"),
	UIOnly UMETA(DisplayName = "UIOnly")
};

/*
 * 설치 액터 타겟팅 정책
 */
UENUM(BlueprintType)
enum class EZoneTargetPolicy : uint8
{
	All UMETA(DisplayName = "All"),
	Friendly UMETA(DisplayName = "Friendly Only"),
	Enemy UMETA(DisplayName = "Enemy Only")
};

UENUM(BlueprintType)
enum class ECMAbilityActivationPolicy : uint8
{
	/* 기본값: 입력 또는 게임플레이 이벤트를 통해 트리거될 때 활성화 */
	OnTriggered,

	/* ASC에 부여(Grant)되는 즉시 자동으로 활성화 (예: 패시브, 1회성 스폰) */
	OnGiven
};

UENUM(BlueprintType)
enum class ERollDirection : uint8
{
	Forward UMETA(DisplayName = "전방 (Forward)"),
	ForwardRight UMETA(DisplayName = "전방 우측 (Forward Right)"),
	Right UMETA(DisplayName = "우측 (Right)"),
	BackwardRight UMETA(DisplayName = "후방 우측 (Backward Right)"),
	Backward UMETA(DisplayName = "후방 (Backward)"),
	BackwardLeft UMETA(DisplayName = "후방 좌측 (Backward Left)"),
	Left UMETA(DisplayName = "좌측 (Left)"),
	ForwardLeft UMETA(DisplayName = "전방 좌측 (Forward Left)")
};