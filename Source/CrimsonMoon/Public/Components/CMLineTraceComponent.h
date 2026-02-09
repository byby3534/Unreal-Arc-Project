// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMLineTraceComponent.generated.h"

class UDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CRIMSONMOON_API UCMLineTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMLineTraceComponent();

	UFUNCTION(BlueprintCallable, Category = "Line Trace")
	bool GetAimHitResultFromLineTrace(FHitResult& OutHitResult, float LineTraceRange, ECollisionChannel TraceChannel = ECC_Visibility) const;
};