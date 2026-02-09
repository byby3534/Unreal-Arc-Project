#pragma once

#include "CoreMinimal.h"
#include "CMAITypes.generated.h"

UENUM(BlueprintType)

enum class ECMEnemyState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Patrol UMETA(DisplayName = "Patrol"),
	Investigating UMETA(DisplayName = "Investigating"),
	Combat UMETA(DisplayName = "Combat"),
	PhaseTransition UMETA(DisplayName = "PhaseTransition"),
	Dead UMETA(DisplayName = "Dead")
};