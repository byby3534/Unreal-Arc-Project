#pragma once

#include "CoreMinimal.h"
#include "Controllers/CMPlayerControllerBase.h"
#include "CMMainMenuPlayerController.generated.h"

UCLASS()
class CRIMSONMOON_API ACMMainMenuPlayerController : public ACMPlayerControllerBase
{
	GENERATED_BODY()

	/* Engine Methods */
protected:
	virtual void BeginPlay() override;
};
