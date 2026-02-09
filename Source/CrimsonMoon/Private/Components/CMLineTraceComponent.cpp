// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CMLineTraceComponent.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"


UCMLineTraceComponent::UCMLineTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


bool UCMLineTraceComponent::GetAimHitResultFromLineTrace(FHitResult& OutHitResult, float LineTraceRange, ECollisionChannel TraceChannel) const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	AController* OwnerController = OwnerPawn->GetController();
	APlayerController* PC = Cast<APlayerController>(OwnerController);

	if (PC)
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

		FVector TraceStart = ViewLocation;
		FVector TraceEnd = TraceStart + (ViewRotation.Vector() * LineTraceRange);

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(OwnerPawn);

		bool bHitDetected = GetWorld()->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, TraceChannel, CollisionParams);
		
		if (!bHitDetected)
		{
			OutHitResult.TraceStart = TraceStart;
			OutHitResult.TraceEnd = TraceEnd;
			OutHitResult.Location = TraceEnd;
		}

		return bHitDetected;
	}

	return false;
}
