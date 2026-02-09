#pragma once

#include "CoreMinimal.h"
#include "Controllers/CMPlayerControllerBase.h"
#include "CMLobbyPlayerController.generated.h"

UCLASS()
class CRIMSONMOON_API ACMLobbyPlayerController : public ACMPlayerControllerBase
{
	GENERATED_BODY()

public:
	ACMLobbyPlayerController();

	virtual void OnPossess(APawn* InPawn) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UCMInteractionUIComponent> InteractionUIComponent;
};
