// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGA_Blink.h"

#include "NavigationSystem.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "GameplayTags/CMGameplayTags_Enemy.h"
#include "GameplayTags/CMGameplayTags_Shared.h"

UCMEnemyGA_Blink::UCMEnemyGA_Blink()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer Tags;
	Tags.AddTag(CMGameplayTags::Enemy_Ability_Blink);
	SetAssetTags(Tags);

	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_HitReact);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Groggy);
}

void UCMEnemyGA_Blink::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(ActorInfo->AvatarActor.Get());

	if (!EnemyCharacter || !BlinkMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		BlinkMontage
		);

	// 몽타주가 끝나거나 중단되면 어빌리티 종료
	MontageTask->OnCompleted.AddDynamic(this, &UCMEnemyGA_Blink::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UCMEnemyGA_Blink::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UCMEnemyGA_Blink::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UCMEnemyGA_Blink::OnMontageCancelled);

	MontageTask->ReadyForActivation();

	if (TeleportEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			TeleportEventTag,
			nullptr,
			false,
			false
			);

		WaitEventTask->EventReceived.AddDynamic(this, &UCMEnemyGA_Blink::OnTeleportEventReceived);
		WaitEventTask->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] TeleportEventTag is invalid!"), *GetName());
	}

}

void UCMEnemyGA_Blink::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMEnemyGA_Blink::OnTeleportEventReceived(FGameplayEventData Payload)
{
	ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo());

	if (!EnemyCharacter)
	{
		return;
	}

	// 타겟 찾기
	const AActor* TargetActor = Payload.Target;

	// TriggerData에 없다면 블랙보드 확인
	if (!TargetActor)
	{
		TargetActor = EnemyCharacter->StateTreeTargetActor;
	}

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] No TargetActor found for Blink. End Ability."), *GetName());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	FVector TargetForwardVector = TargetActor->GetActorForwardVector();
	TargetForwardVector.Z = 0.0f;
	TargetForwardVector.Normalize();
	const FVector TargetLocation = TargetActor->GetActorLocation();

	// 타겟 등 뒤 위치
	const FVector DesiredLocationBehindTarget = TargetLocation - (TargetForwardVector * DistanceBehindTargetToTeleport);

	// 벽 속이나 허공으로 이동하는 것을 방지
	FVector FinalTeleportLocation = DesiredLocationBehindTarget;
	if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation NavLocation;
		// 검색 반경
		if (NavSystem->ProjectPointToNavigation(DesiredLocationBehindTarget, NavLocation, FVector(200.f, 200.f, 200.f)))
		{
			FinalTeleportLocation = NavLocation.Location;
			FinalTeleportLocation.Z += EnemyCharacter->GetSimpleCollisionHalfHeight();
		}
	}

	// 위치 이동 및 회전
	FRotator LookAtTargetRot = (TargetLocation - FinalTeleportLocation).Rotation();
	LookAtTargetRot.Pitch = 0.f; // 평지 회전만

	EnemyCharacter->TeleportTo(FinalTeleportLocation, LookAtTargetRot);
}

void UCMEnemyGA_Blink::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMEnemyGA_Blink::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}