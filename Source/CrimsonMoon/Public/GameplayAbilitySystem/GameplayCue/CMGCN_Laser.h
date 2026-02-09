// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "CMGCN_Laser.generated.h"

class UNiagaraComponent;

UCLASS()
class CRIMSONMOON_API ACMGCN_Laser : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	ACMGCN_Laser();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

	void UpdateLaserTransform(float DeltaTime);

	UPROPERTY(VisibleAnywhere, Category = "VFX")
	TObjectPtr<UNiagaraComponent> LaserNiagaraComponent;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	FName MuzzleSocketName = FName("Yuna_HandGrip_R");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Niagara")
	FName LaserLengthParamName = FName("User.Length");

	UPROPERTY(EditDefaultsOnly, Category = "VFX|Niagara")
	FName LaserDurationParamName = FName("User.Duration");

private:
	/** 목표 지점 (서버로부터 업데이트됨) */
	FVector TargetBeamEndLocation;

	FVector CurrentSmoothEndLocation;

	bool bAllowAutoRestart = false;

	float CachedLaserDuration = 1.0f;

	UFUNCTION()
	void OnLaserSystemFinished(UNiagaraComponent* FinishedComponent);
};