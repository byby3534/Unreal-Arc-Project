// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CMNpcBase.h"
#include "CMNpcGameStarter.generated.h"

UCLASS()
class CRIMSONMOON_API ACMNpcGameStarter : public ACMNpcBase
{
	GENERATED_BODY()

public:
	ACMNpcGameStarter();

protected:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void PerformInteract() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameStart")
	FString TravelURL;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameStart")
	float FadeDuration = 1.0f;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartFadeOut();

private:
	void ServerTravelToConfiguredMap();

};