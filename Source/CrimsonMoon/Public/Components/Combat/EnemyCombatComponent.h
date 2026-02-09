// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnCombatComponent.h"
#include "EnemyCombatComponent.generated.h"


class UBoxComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMSONMOON_API UEnemyCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	UEnemyCombatComponent();

	#pragma region Hand Collision
	UFUNCTION(BlueprintCallable, Category = "Crimson Moon | Combat")
	void ToggleHandCollision(const bool bEnable, EToggleDamageType ToggleDamageType);

	#pragma endregion

protected:
	virtual void HandleToggleCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType) override;

	/**
	 * @brief 타겟을 타격했을 때 추가 로직 처리 (블록/패링 판정)
	 * @param HitActor 타격한 액터
	 */
	virtual void OnHitTargetActorImpl(AActor* HitActor) override;
};