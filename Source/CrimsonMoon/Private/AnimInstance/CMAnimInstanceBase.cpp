// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimInstance/CMAnimInstanceBase.h"
#include "CMFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Character/CMCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCMAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningCharacter = Cast<ACMCharacterBase>(TryGetPawnOwner());

	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void UCMAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}

	GroundSpeed = OwningCharacter->GetVelocity().Size2D();

	IsFalling = OwningMovementComponent->IsFalling();

	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER;
}

bool UCMAnimInstanceBase::DoesOwnerHaveTag(FGameplayTag TagToCheck) const
{
	if (APawn* OwningPawn = TryGetPawnOwner())
	{
		return UCMFunctionLibrary::NativeDoesActorHaveTag(OwningPawn, TagToCheck);
	}

	return false;
}