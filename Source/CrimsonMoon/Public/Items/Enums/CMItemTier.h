// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CMItemTier.generated.h"

UENUM(BlueprintType)
enum class ECMItemTier : uint8
{
	Rusted		UMETA(DisplayName = "Rusted"),
	Standard	UMETA(DisplayName = "Standard"),
	Reinforced	UMETA(DisplayName = "Reinforced"),
	ARC			UMETA(DisplayName = "ARC")
};