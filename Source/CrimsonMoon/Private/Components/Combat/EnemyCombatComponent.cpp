// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/EnemyCombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "Components/BoxComponent.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "Character/Player/CMPlayerCharacterBase.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
	SetIsReplicatedByDefault(true);
}

void UEnemyCombatComponent::ToggleHandCollision(const bool bEnable, EToggleDamageType ToggleDamageType)
{
	const ACMEnemyCharacterBase* EnemyCharacterBase = GetOwningPawn<ACMEnemyCharacterBase>();
	if (!EnemyCharacterBase)
	{
		return;
	}

	UBoxComponent* LeftHandCollisionBox = EnemyCharacterBase->GetLeftHandCollisionBox();
	UBoxComponent* RightHandCollisionBox = EnemyCharacterBase->GetRightHandCollisionBox();

	const ECollisionEnabled::Type CollisionQuery = bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;

	switch (ToggleDamageType)
	{
		case EToggleDamageType::LeftHand:
			LeftHandCollisionBox->SetCollisionEnabled(CollisionQuery);
			break;

		case EToggleDamageType::RightHand:
			RightHandCollisionBox->SetCollisionEnabled(CollisionQuery);
			break;

		default:
			break;
	}

	// 콜리전 비활성화 시 OverlappedActors 초기화
	if (!bEnable)
	{
		OverlappedActors.Empty();
	}
}

void UEnemyCombatComponent::HandleToggleCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// LeftHand 또는 RightHand인 경우 ToggleHandCollision 호출
	if (ToggleDamageType == EToggleDamageType::LeftHand || ToggleDamageType == EToggleDamageType::RightHand)
	{
		ToggleHandCollision(bShouldEnable, ToggleDamageType);
	}
	else
	{
		// CurrentEquippedWeapon인 경우 부모 클래스 로직 사용
		Super::HandleToggleCollision(bShouldEnable, ToggleDamageType);
	}
}

void UEnemyCombatComponent::OnHitTargetActorImpl(AActor* HitActor)
{
	Super::OnHitTargetActorImpl(HitActor);

	if (!HitActor)
	{
		return;
	}

	bool bIsValidBlock = false;

	if (const bool bIsPlayerBlocking = UCMFunctionLibrary::NativeDoesActorHaveTag(HitActor, CMGameplayTags::Player_Status_Blocking))
	{
		bIsValidBlock = UCMFunctionLibrary::IsValidBlock(GetOwningPawn(), HitActor);
	}

	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn(); // 공격자 (Enemy)
	EventData.Target = HitActor;
	EventData.EventMagnitude = GetWorld()->GetTimeSeconds(); // 공격 시점 전달 (퍼펙트 블록 판정용)

	if (bIsValidBlock)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			HitActor,
			CMGameplayTags::Player_Event_SuccessfulBlock,
			EventData
			);
	}
}