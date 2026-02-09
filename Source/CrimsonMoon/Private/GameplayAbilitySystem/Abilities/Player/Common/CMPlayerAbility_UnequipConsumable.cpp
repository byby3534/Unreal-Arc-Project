// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_UnequipConsumable.h"
#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_EquipConsumable.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "CMGameplayTags.h"

UCMPlayerAbility_UnequipConsumable::UCMPlayerAbility_UnequipConsumable()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Shared_Ability_Death);
	SetAssetTags(TagsToAdd);
}

void UCMPlayerAbility_UnequipConsumable::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 장착 중인 소모품 어빌리티를 취소
	FGameplayTagContainer TargetTags;
	TargetTags.AddTag(CMGameplayTags::Player_Ability_EquipConsumable);
	ASC->CancelAbilities(&TargetTags);

	// 메쉬 강제 제거
	ACMPlayerCharacterBase* Character = Cast<ACMPlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (Character && Character->GetHandHeldItemMesh())
	{
		Character->GetHandHeldItemMesh()->SetStaticMesh(nullptr);
		Character->GetHandHeldItemMesh()->SetVisibility(false);
		
		// 소켓 위치 초기화
		if (Character->GetHandHeldItemMesh()->GetAttachSocketName() != FName("Hand_Item"))
		{
			Character->GetHandHeldItemMesh()->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Hand_Item"));
		}
	}

	// 부여된 하위 어빌리티 제거 (서버에서만)
	if (HasAuthority(&ActivationInfo))
	{
		FGameplayTag ParentTag = CMGameplayTags::Player_Ability_Item;

		TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.GetDynamicSpecSourceTags().HasTag(ParentTag))
			{
				AbilitiesToRemove.Add(Spec.Handle);
			}
		}

		for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitiesToRemove)
		{
			ASC->ClearAbility(SpecHandle);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
