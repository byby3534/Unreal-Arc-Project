// Fill out your copyright notice in the Description page of Project Settings.


#include "CrimsonMoon/Public/AnimNotifies/AN_ToggleCollision.h"
#include "CMFunctionLibrary.h"
#include "Interfaces/PawnCombatInterface.h"

void UAN_ToggleCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}
	
	AActor* OwnerActor = MeshComp->GetOwner();	
	
	if (!OwnerActor)
	{
		return;
	}
	
	ECMValidType ValidType;
	if (UPawnCombatComponent* PawnCombatComponent = UCMFunctionLibrary::BP_GetPawnCombatComponentFromActor(OwnerActor, ValidType))
	{
		if (ValidType == ECMValidType::Valid)
		{
			PawnCombatComponent->ToggleWeaponCollision(true, DamageTypeToToggle);
		}
	}
}

void UAN_ToggleCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;		
	}

	AActor* OwnerActor = MeshComp->GetOwner();

	if (!OwnerActor)
	{
		return;
	}

	ECMValidType ValidType;
	if (UPawnCombatComponent* CombatComp = UCMFunctionLibrary::BP_GetPawnCombatComponentFromActor(OwnerActor, ValidType))
	{
		if (ValidType == ECMValidType::Valid)
		{
			CombatComp->ToggleWeaponCollision(false, DamageTypeToToggle);
		}
	}
}