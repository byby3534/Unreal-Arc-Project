// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Interfaces/PawnCombatInterface.h"
#include "Interfaces/PawnUIInterface.h"
#include "CMCharacterBase.generated.h"

class UBoxComponent;
class UCMDataAsset_StartupDataBase;
class UCMAttributeSet;
class UCMAbilitySystemComponent;
class UAnimMontage;

UCLASS()
class CRIMSONMOON_API ACMCharacterBase
	: public ACharacter, public IAbilitySystemInterface, public IPawnCombatInterface, public IPawnUIInterface

{
	GENERATED_BODY()

public:
	ACMCharacterBase();
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnMoveSpeedAttributeChanged(const FOnAttributeChangeData& Data);

	#pragma region Interfaces

	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UPawnUIComponent* GetPawnUIComponent() const override;

	#pragma endregion

protected:
	virtual void PossessedBy(AController* NewController) override;

	#pragma region GAS

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<UCMAbilitySystemComponent> CMAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<UCMAttributeSet> CMAttributeSet;

	#pragma endregion

	#pragma region Interface Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UPawnCombatComponent> PawnCombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPawnUIComponent> PawnUIComponent;
	#pragma endregion

	#pragma region DataAssets

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData | DataAsset")
	TSoftObjectPtr<UCMDataAsset_StartupDataBase> CharacterStartUpData;

	#pragma endregion

public:
	#pragma region FORCEINLINE

	FORCEINLINE UCMAbilitySystemComponent* GetCMAbilitySystemComponent() const { return CMAbilitySystemComponent; }
	FORCEINLINE UCMAttributeSet* GetCMAttributeSet() const { return CMAttributeSet; }
	FORCEINLINE UPawnUIComponent* GetCMPawnUIComponent() const { return PawnUIComponent; }

	#pragma endregion

	#pragma region Death Animation

	/** 모든 클라이언트에서 사망 몽타주 재생 (NetMulticast) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathMontage(UAnimMontage* MontageToPlay);

	#pragma endregion
};