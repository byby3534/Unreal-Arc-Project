// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerGameplayAbility_TargetLock.h"
#include "CMGameplayTags.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"

UCMPlayerGameplayAbility_TargetLock::UCMPlayerGameplayAbility_TargetLock()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_TargetLock);

	SetAssetTags(TagsToAdd);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UCMPlayerGameplayAbility_TargetLock::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();
	UPlayerCombatComponent* CombatComp = OwnerCharacter ? OwnerCharacter->GetPawnCombatComponent() : nullptr;
	const UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();

	if (!OwnerCharacter || !CombatComp || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 락온 활성화
	if (AActor* BestTarget = CombatComp->FindBestLockOnTarget())
	{
		Server_SetLockOnState(true, BestTarget); // 클라 -> 서버 요청

		if (OwnerCharacter->IsLocallyControlled())
		{
			// 클라이언트 예측
			ApplyLockOnState(true, BestTarget);
		}
		// 어빌리티가 Active 상태를 유지해야 하므로 EndAbility() 호출 안 함
	}
	else
	{
		// 타겟을 못찾았으므로 즉시 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UCMPlayerGameplayAbility_TargetLock::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// ASC에서 CancelAbility가 호출되면 서버/클라 모두에서 EndAbility가 호출됨
	ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();

	// 서버에서만 RPC 호출
	if (OwnerCharacter && HasAuthority(&ActivationInfo))
	{
		Server_SetLockOnState(false, nullptr);
	}

	// 로컬 클라이언트 예측
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		ApplyLockOnState(false, nullptr);
	}
}

bool UCMPlayerGameplayAbility_TargetLock::Server_SetLockOnState_Validate(bool bShouldLockOn, AActor* NewTarget)
{
	return true;
}

void UCMPlayerGameplayAbility_TargetLock::Server_SetLockOnState_Implementation(bool bShouldLockOn, AActor* NewTarget)
{
	ApplyLockOnState(bShouldLockOn, NewTarget);
}

void UCMPlayerGameplayAbility_TargetLock::ApplyLockOnState(bool bShouldLockOn, AActor* NewTarget) const
{
	ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();
	UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();
	UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter ? OwnerCharacter->GetPawnCombatComponent() : nullptr;
	UCharacterMovementComponent* MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;

	if (!OwnerCharacter || !PlayerCombatComponent || !ASC || !MovementComponent)
	{
		return;
	}

	// CurrentLockOnTarget은 서버에서 설정하면 자동으로 복제되지만 클라이언트 예측을 위해 클라이언트에서도 즉시 설정
	PlayerCombatComponent->CurrentLockOnTarget = NewTarget;

	// UI 업데이트를 위한 델리게이트 브로드캐스트
	if (PlayerCombatComponent->OnLockOnTargetChanged.IsBound())
	{
		PlayerCombatComponent->OnLockOnTargetChanged.Broadcast(bShouldLockOn, NewTarget);
	}

	if (bShouldLockOn)
	{
		// 락온 설정값
		ASC->AddLooseGameplayTag(CMGameplayTags::Shared_Status_IsLockedOn);

		MovementComponent->MaxWalkSpeed = OwnerCharacter->GetTargetLockMaxWalkSpeed();
		MovementComponent->bOrientRotationToMovement = false;
		OwnerCharacter->bUseControllerRotationYaw = true;
	}
	else
	{
		// 락온 해제
		ASC->RemoveLooseGameplayTag(CMGameplayTags::Shared_Status_IsLockedOn);

		MovementComponent->MaxWalkSpeed = OwnerCharacter->GetCachedDefaultMaxWalkSpeed();
		MovementComponent->bOrientRotationToMovement = true;
		OwnerCharacter->bUseControllerRotationYaw = false;
	}
}