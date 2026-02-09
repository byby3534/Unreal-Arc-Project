// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "PlayerCombatComponent.generated.h"

class UCMDataAsset_WeaponData;
class ACMPlayerWeapon;

// 락온 타겟 변경 시 호출되는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLockOnTargetChanged, bool, bIsLockedOn, AActor*, Target);

UCLASS()
class CRIMSONMOON_API UPlayerCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	#pragma region Core & Overrides
	UPlayerCombatComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnWeaponPulledFromTargetActor(AActor* InteractingActor) override;
	#pragma endregion

	#pragma region LockOn Public API
	UPROPERTY(BlueprintAssignable, Category = "Crimson Moon | LockOn")
	FOnLockOnTargetChanged OnLockOnTargetChanged;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Crimson Moon | LockOn")
	TWeakObjectPtr<AActor> CurrentLockOnTarget;

	UFUNCTION(BlueprintCallable, Category = "Crimson Moon | LockOn")
	AActor* FindBestLockOnTarget();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Crimson Moon | LockOn")
	void Server_SwitchLockOnTarget(FVector2D LookAxis);
	#pragma endregion

	#pragma region Debug Public API
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Crimson Moon | LockOn | Debug")
	void Server_ToggleLockOnDebugTrace(bool bShouldShow);

	UFUNCTION(BlueprintPure, Category = "Crimson Moon | LockOn | Debug")
	bool GetShowLockOnDebugTrace() const { return bShowLockOnDebugTrace; }
	#pragma endregion

	#pragma region Weapon Public API
	ACMPlayerWeapon* GetPlayerCarriedWeaponByTag(FGameplayTag InWeaponTag) const;
	ACMPlayerWeapon* GetPlayerCurrentEquippedWeapon() const;
	const UCMDataAsset_WeaponData* GetPlayerCurrentWeaponData() const;

	UFUNCTION(BlueprintPure, Category = "Crimson Moon | Weapon|Damage")
	float GetPlayerCurrentEquippedWeaponDamageAtLevel(float InLevel) const;

	UFUNCTION(BlueprintPure, Category = "Crimson Moon | Weapon|Groggy")
	float GetPlayerCurrentWeaponHeavyGroggyDamage(float InLevel) const;

	UFUNCTION(BlueprintPure, Category = "Crimson Moon | Weapon|Groggy")
	float GetPlayerCurrentWeaponCounterGroggyDamage(float InLevel) const;
	#pragma endregion

protected:
	#pragma region LockOn Settings
	UPROPERTY(EditDefaultsOnly, Category = "Crimson Moon | LockOn")
	float LockOnBoxTraceDistance = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Crimson Moon | LockOn")
	FVector LockOnTraceBoxSize = FVector(1000.f, 1000.f, 300.f);

	UPROPERTY(EditDefaultsOnly, Category = "Crimson Moon | LockOn")
	TArray<TEnumAsByte<EObjectTypeQuery>> LockOnTraceChannel;
	#pragma endregion

	#pragma region Debug Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Crimson Moon | Debug")
	bool bShowLockOnDebugTrace = false;
	#pragma endregion

private:
	#pragma region Internal Helpers
	void GetAvailableLockOnTargets(TArray<AActor*>& OutAvailableActors) const;
	#pragma endregion
};