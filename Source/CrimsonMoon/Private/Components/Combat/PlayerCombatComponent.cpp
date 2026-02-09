// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PlayerCombatComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "DataAssets/Weapon/CMDataAsset_WeaponData.h"
#include "Items/Weapons/CMPlayerWeapon.h"
#include "Items/Weapons/CMWeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	LockOnTraceChannel.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	SetIsReplicatedByDefault(true);
}

void UPlayerCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPlayerCombatComponent, CurrentLockOnTarget);
	DOREPLIFETIME_CONDITION(UPlayerCombatComponent, bShowLockOnDebugTrace, COND_OwnerOnly);
}

void UPlayerCombatComponent::OnWeaponPulledFromTargetActor(AActor* InteractingActor)
{
	Super::OnWeaponPulledFromTargetActor(InteractingActor);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		CMGameplayTags::Player_Event_HitPause,
		FGameplayEventData()
		);
}

void UPlayerCombatComponent::Server_ToggleLockOnDebugTrace_Implementation(bool bShouldShow)
{
	bShowLockOnDebugTrace = bShouldShow;
}

ACMPlayerWeapon* UPlayerCombatComponent::GetPlayerCarriedWeaponByTag(FGameplayTag InWeaponTag) const
{
	return Cast<ACMPlayerWeapon>(GetCharacterCarriedWeaponByTag(InWeaponTag));
}

ACMPlayerWeapon* UPlayerCombatComponent::GetPlayerCurrentEquippedWeapon() const
{
	ACMPlayerWeapon* PlayerWeapon = Cast<ACMPlayerWeapon>(GetCharacterCurrentEquippedWeapon());

	return PlayerWeapon ? PlayerWeapon : nullptr;
}

const UCMDataAsset_WeaponData* UPlayerCombatComponent::GetPlayerCurrentWeaponData() const
{
	if (ACMPlayerWeapon* PlayerWeapon = GetPlayerCurrentEquippedWeapon())
	{
		return PlayerWeapon->WeaponData;
	}
	return nullptr;
}

float UPlayerCombatComponent::GetPlayerCurrentEquippedWeaponDamageAtLevel(float InLevel) const
{
	if (const ACMPlayerWeapon* PlayerWeapon = GetPlayerCurrentEquippedWeapon())
	{
		if (const UCMDataAsset_WeaponData* Weapon = PlayerWeapon->WeaponData)
		{
			return Weapon->WeaponBaseDamage.GetValueAtLevel(InLevel);
		}
	}

	return 0.f;
}

float UPlayerCombatComponent::GetPlayerCurrentWeaponHeavyGroggyDamage(float InLevel) const
{
	if (const UCMDataAsset_WeaponData* WeaponData = GetPlayerCurrentWeaponData())
	{
		return WeaponData->HeavyAttackGroggyDamage.GetValueAtLevel(InLevel);
	}
	return 0.f;
}

float UPlayerCombatComponent::GetPlayerCurrentWeaponCounterGroggyDamage(float InLevel) const
{
	if (const UCMDataAsset_WeaponData* WeaponData = GetPlayerCurrentWeaponData())
	{
		return WeaponData->CounterAttackGroggyDamage.GetValueAtLevel(InLevel);
	}
	return 0.f;
}

AActor* UPlayerCombatComponent::FindBestLockOnTarget()
{
	ACMPlayerCharacterBase* OwnerCharacter = Cast<ACMPlayerCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return nullptr;
	}

	TArray<AActor*> AvailableActorsToLock;
	GetAvailableLockOnTargets(AvailableActorsToLock);

	if (AvailableActorsToLock.IsEmpty())
	{
		return nullptr;
	}

	float ClosestDistance = 0.f;
	return UGameplayStatics::FindNearestActor(OwnerCharacter->GetActorLocation(), AvailableActorsToLock, ClosestDistance);
}

void UPlayerCombatComponent::Server_SwitchLockOnTarget_Implementation(FVector2D LookAxis)
{
	if (!CurrentLockOnTarget.IsValid())
	{
		return;
	}

	TArray<AActor*> AvailableActorsToLock;
	GetAvailableLockOnTargets(AvailableActorsToLock);

	if (AvailableActorsToLock.Num() <= 1)
	{
		return;
	}

	TArray<AActor*> ActorsOnLeft;
	TArray<AActor*> ActorsOnRight;

	const FVector PlayerLocation = GetOwner()->GetActorLocation();
	const FVector PlayerToCurrentNormalized = (CurrentLockOnTarget->GetActorLocation() - PlayerLocation).GetSafeNormal();

	for (AActor* AvailableActor : AvailableActorsToLock)
	{
		if (!AvailableActor || AvailableActor == CurrentLockOnTarget.Get())
		{
			continue;
		}

		const FVector PlayerToAvailableNormalized = (AvailableActor->GetActorLocation() - PlayerLocation).GetSafeNormal();
		const FVector CrossResult = FVector::CrossProduct(PlayerToCurrentNormalized, PlayerToAvailableNormalized);

		if (CrossResult.Z > 0.f)
		{
			ActorsOnRight.AddUnique(AvailableActor);
		}
		else
		{
			ActorsOnLeft.AddUnique(AvailableActor);
		}
	}

	float ClosestDistance = 0.f;
	AActor* NewTargetToLock = nullptr;

	if (LookAxis.X > 0.5f && !ActorsOnRight.IsEmpty()) // 오른쪽
	{
		NewTargetToLock = UGameplayStatics::FindNearestActor(PlayerLocation, ActorsOnRight, ClosestDistance);
	}
	else if (LookAxis.X < -0.5f && !ActorsOnLeft.IsEmpty()) // 왼쪽
	{
		NewTargetToLock = UGameplayStatics::FindNearestActor(PlayerLocation, ActorsOnLeft, ClosestDistance);
	}

	if (NewTargetToLock)
	{
		CurrentLockOnTarget = NewTargetToLock;

		// UI 업데이트를 위한 델리게이트 브로드캐스트 (타겟 전환)
		if (OnLockOnTargetChanged.IsBound())
		{
			OnLockOnTargetChanged.Broadcast(true, NewTargetToLock);
		}
	}
}

void UPlayerCombatComponent::GetAvailableLockOnTargets(TArray<AActor*>& OutAvailableActors) const
{
	OutAvailableActors.Empty();
	ACMPlayerCharacterBase* OwnerCharacter = Cast<ACMPlayerCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	TArray<FHitResult> BoxTraceHits;
	const FVector TraceStart = OwnerCharacter->GetActorLocation();
	const FVector TraceEnd = TraceStart + OwnerCharacter->GetActorForwardVector() * LockOnBoxTraceDistance;
	const FVector HalfSize = LockOnTraceBoxSize / 2.f;
	const FRotator Orientation = OwnerCharacter->GetActorForwardVector().ToOrientationRotator();

	const EDrawDebugTrace::Type DebugDrawType = bShowLockOnDebugTrace
		? EDrawDebugTrace::ForDuration
		: EDrawDebugTrace::None;

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		TraceStart,
		TraceEnd,
		HalfSize,
		Orientation,
		LockOnTraceChannel,
		false,
		TArray<AActor*>(),
		DebugDrawType,
		BoxTraceHits,
		true
		);

	for (const FHitResult& TraceHit : BoxTraceHits)
	{
		AActor* HitActor = TraceHit.GetActor();

		// Enemy 캐릭터인지 확인
		const ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(HitActor);
		if (!EnemyCharacter)
		{
			continue;
		}

		const bool IsDeadTarget = UCMFunctionLibrary::NativeDoesActorHaveTag(HitActor, CMGameplayTags::Shared_Status_Dead);
		// Enemy이고, 죽지 않은 액터만 타겟으로 추가
		if (!IsDeadTarget)
		{
			OutAvailableActors.AddUnique(HitActor);
		}
	}
}