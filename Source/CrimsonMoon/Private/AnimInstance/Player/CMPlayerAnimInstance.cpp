// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimInstance/Player/CMPlayerAnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "CrimsonMoon/DebugHelper.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCMPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPlayerCharacter = Cast<ACMPlayerCharacterBase>(OwningCharacter);

	if (OwningCharacter)
	{
		LastActorRotation = OwningCharacter->GetActorRotation();
	}
}

void UCMPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningPlayerCharacter || !OwningMovementComponent)
	{
		return;
	}

	IsCrouching = OwningMovementComponent->IsCrouching();

	// 이동 방향 계산
	if (GroundSpeed > KINDA_SMALL_NUMBER)
	{
		MoveDirection = UKismetAnimationLibrary::CalculateDirection(
			OwningCharacter->GetVelocity(),
			OwningCharacter->GetActorRotation());
	}
	else
	{
		MoveDirection = 0.f; // 멈춰있으면 방향 없음
	}

	// Turn in Place 계산
	UpdateTurnInPlace(DeltaSeconds);
}

void UCMPlayerAnimInstance::UpdateTurnInPlace(float DeltaSeconds)
{
	if (!OwningCharacter)
	{
		return;
	}

	const FRotator CurrentActorRotation = OwningCharacter->GetActorRotation();

	if (GroundSpeed < KINDA_SMALL_NUMBER)
	{
		const float DeltaYaw = FMath::FindDeltaAngleDegrees(LastActorRotation.Yaw, CurrentActorRotation.Yaw);

		RootYawOffset = FMath::FInterpTo(RootYawOffset, RootYawOffset + DeltaYaw, DeltaSeconds, YawDeltaInterpSpeed);

		YawDelta = RootYawOffset;

		if (FMath::Abs(RootYawOffset) > 90.f)
		{
			RootYawOffset = 0.f;
		}
	}
	else
	{
		RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.f, DeltaSeconds, YawDeltaInterpSpeed);
		YawDelta = 0.f;
	}

	LastActorRotation = CurrentActorRotation;
}