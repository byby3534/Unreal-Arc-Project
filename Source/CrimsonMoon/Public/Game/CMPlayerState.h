

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CMPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API ACMPlayerState : public APlayerState
{
	GENERATED_BODY()

	/* Engine Methods */

	/* Custom Methods */
public:
	FORCEINLINE void SetIsPlayerDead(bool bDead) { bIsPlayerDead = bDead; }
	FORCEINLINE bool GetIsPlayerDead() const { return bIsPlayerDead; }

protected:
	bool bIsPlayerDead = false;

};


