// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Common/CMAbility_Groggy.h"
#include "CMGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Character/CMCharacterBase.h"
#include "GameplayAbilitySystem/CMAttributeSet.h"

UCMAbility_Groggy::UCMAbility_Groggy()
{
	// Ability 태그 설정
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Shared_Ability_Groggy);
	SetAssetTags(TagsToAdd);

	// 이벤트 트리거 설정
	AbilityTriggers.SetNum(1);
	AbilityTriggers[0].TriggerTag = CMGameplayTags::Shared_Event_GroggyTriggered;
	AbilityTriggers[0].TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	// 그로기 중에는 다시 그로기 상태가 되지 않도록
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Groggy);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UCMAbility_Groggy::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	const ACMCharacterBase* CharacterBase = GetCMCharacterFromActorInfo();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !CharacterBase)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 움직임 제한
	if (UCharacterMovementComponent* MoveComp = CharacterBase->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(MOVE_None);
	}

	// 그로기 상태 GE 적용 (Shared.Status.Groggy 태그 부여)
	// GE의 Duration은 블루프린트에서 설정됨
	// GE의 Conditional GameplayEffects를 통해 만료 시 자동으로 CurrentGroggy 리셋됨
	if (GroggyStateEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			GroggyStateEffect,
			GetAbilityLevel(),
			EffectContext
			);

		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Shared.Status.Groggy 태그가 제거될 때까지 대기
	TagEventHandle = ASC->RegisterGameplayTagEvent(
		CMGameplayTags::Shared_Status_Groggy,
		EGameplayTagEventType::NewOrRemoved // 태그 추가 & 제거 감지
		).AddUObject(this, &ThisClass::OnGroggyTagChanged);
}

void UCMAbility_Groggy::OnGroggyTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 태그가 제거되었을 때 (NewCount == 0)
	if (NewCount == 0)
	{
		OnGroggyEnd();
	}
}

void UCMAbility_Groggy::OnGroggyEnd()
{
	// 델리게이트 해제
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (TagEventHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(CMGameplayTags::Shared_Status_Groggy).Remove(TagEventHandle);
			TagEventHandle.Reset();
		}
	}

	// 움직임 복구
	if (const ACMCharacterBase* CharacterBase = GetCMCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* MoveComp = CharacterBase->GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}