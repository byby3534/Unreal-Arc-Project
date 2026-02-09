// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/CMCharacterBase.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "DataAssets/Startup/CMDataAsset_StartupDataBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "GameplayAbilitySystem/CMAttributeSet.h"

ACMCharacterBase::ACMCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PawnUIComponent = CreateDefaultSubobject<UPawnUIComponent>(TEXT("Pawn UI Component"));

	CMAbilitySystemComponent = CreateDefaultSubobject<UCMAbilitySystemComponent>(TEXT("Ability System Component"));
	CMAbilitySystemComponent->SetIsReplicated(true);
	CMAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CMAttributeSet = CreateDefaultSubobject<UCMAttributeSet>(TEXT("Attribute Set"));
}

UPawnCombatComponent* ACMCharacterBase::GetPawnCombatComponent() const
{
	return nullptr;
}

UAbilitySystemComponent* ACMCharacterBase::GetAbilitySystemComponent() const
{
	return GetCMAbilitySystemComponent();
}

UPawnUIComponent* ACMCharacterBase::GetPawnUIComponent() const
{
	return PawnUIComponent;
}

void ACMCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!CharacterStartUpData.IsNull())
	{
		if (UCMDataAsset_StartupDataBase* LoadedData = CharacterStartUpData.LoadSynchronous())
		{
			constexpr int32 AbilityApplyLevel = 1;
			LoadedData->GiveToAbilitySystemComponent(CMAbilitySystemComponent, AbilityApplyLevel);
		}
	}

	if (CMAbilitySystemComponent && CMAttributeSet)
	{
		CMAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(CMAttributeSet->GetMoveSpeedAttribute()).AddUObject(
			this, &ACMCharacterBase::OnMoveSpeedAttributeChanged);

		if (GetCharacterMovement())
		{
			const float InitialMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;
			CMAbilitySystemComponent->SetNumericAttributeBase(CMAttributeSet->GetMoveSpeedAttribute(), InitialMoveSpeed);
		}
	}
}

// Called to bind functionality to input
void ACMCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACMCharacterBase::OnMoveSpeedAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void ACMCharacterBase::Multicast_PlayDeathMontage_Implementation(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay)
	{
		return;
	}

	// 모든 클라이언트(서버 포함)에서 몽타주 재생
	PlayAnimMontage(MontageToPlay);
}