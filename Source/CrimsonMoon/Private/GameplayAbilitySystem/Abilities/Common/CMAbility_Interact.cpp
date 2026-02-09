// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Common/CMAbility_Interact.h"
#include "Interfaces/CMInteractableInterface.h"
#include "Components/Input/CMPickUpComponent.h"


UCMAbility_Interact::UCMAbility_Interact()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCMAbility_Interact::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 아바타 액터와 필요한 컴포넌트를 가져옵니다.
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 아바타(캐릭터)가 PickUpComponent 찾음
	UCMPickUpComponent* PickUpComp = AvatarActor->FindComponentByClass<UCMPickUpComponent>();
	if (!PickUpComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 컴포넌트를 활용해 트레이스 실행
	AActor* HitActor = PickUpComp->GetCurrentInteractable();

	// 타겟이 유효하면 상호작용 실행
	if (HitActor && HitActor->Implements<UCMInteractableInterface>())
	{
		ICMInteractableInterface::Execute_Interact(HitActor, AvatarActor);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
