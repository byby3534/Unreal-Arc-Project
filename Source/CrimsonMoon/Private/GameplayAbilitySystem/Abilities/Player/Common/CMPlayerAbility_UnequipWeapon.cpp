// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_UnequipWeapon.h"
#include "CMGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "Character/CMCharacterBase.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Items/Weapons/CMPlayerWeapon.h"

UCMPlayerAbility_UnequipWeapon::UCMPlayerAbility_UnequipWeapon()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_UnEquipWeapon);
	SetAssetTags(TagsToAdd);

	// 장착 중에는 장착/해제 어빌리티 동시 실행 방지
	ActivationBlockedTags.AddTag(CMGameplayTags::Player_Status_Equipping);
	ActivationBlockedTags.AddTag(CMGameplayTags::Player_Status_Charging);
	ActivationOwnedTags.AddTag(CMGameplayTags::Player_Status_Equipping);
	// ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;

	UnequipEventTag = CMGameplayTags::Player_Event_UnequipWeapon;
}

bool UCMPlayerAbility_UnequipWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ACMCharacterBase* OwnerCharacter = GetCMCharacterFromActorInfo();
	if (!OwnerCharacter)
	{
		return false;
	}

	const UPawnCombatComponent* CombatComponent = OwnerCharacter->GetPawnCombatComponent();
	if (!CombatComponent)
	{
		return false;
	}

	// 무기를 들고 있어야 활성화 가능
	return CombatComponent->GetCharacterCurrentEquippedWeapon() != nullptr;
}


void UCMPlayerAbility_UnequipWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 몽타주가 없으면, 로직만 즉시 실행하고 어빌리티를 종료
	if (!UnequipMontage)
	{
		HandleUnequipLogic();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 몽타주 재생 태스크 생성
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, UnequipMontage);
	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
	PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);

	// 게임플레이 이벤트 수신 대기 태스크 생성
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UnequipEventTag);
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);

	PlayMontageTask->ReadyForActivation();
	WaitEventTask->ReadyForActivation();
}

void UCMPlayerAbility_UnequipWeapon::OnMontageEnded()
{
	// HandleUnequipLogic();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_UnequipWeapon::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMPlayerAbility_UnequipWeapon::OnEventReceived(FGameplayEventData Payload)
{
	// [서버 전용] 이벤트 태그를 수신하면 실제 해제 로직을 처리
	if (HasAuthority(&CurrentActivationInfo))
	{
		HandleUnequipLogic();
	}
}

void UCMPlayerAbility_UnequipWeapon::HandleUnequipLogic()
{
	// 서버에서만 실행
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	const ACMCharacterBase* OwnerCharacter = GetCMCharacterFromActorInfo();
	if (!OwnerCharacter)
	{
		return;
	}

	UPawnCombatComponent* PawnCombatComponent = OwnerCharacter->GetPawnCombatComponent();
	UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();

	if (!PawnCombatComponent || !ASC)
	{
		return;
	}

	if (ACMPlayerWeapon* PlayerWeapon = Cast<ACMPlayerWeapon>(PawnCombatComponent->GetCharacterCurrentEquippedWeapon()))
	{
		// 데이터 에셋 포인터를 가져오고 nullptr 체크
		if (const UCMDataAsset_WeaponData* WeaponData = PlayerWeapon->WeaponData)
		{
			// 1. 서버: 능력 제거
			const TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles = PlayerWeapon->GetGrantedAbilitySpecHandles();

			for (const FGameplayAbilitySpecHandle& SpecHandle : GrantedAbilitySpecHandles)
			{
				ASC->ClearAbility(SpecHandle);
			}

			PlayerWeapon->AssignGrantedAbilitySpecHandles(TArray<FGameplayAbilitySpecHandle>());

			// 2. 서버: 상태 변경 (OnRep 호출)
			PawnCombatComponent->SetCurrentEquippedWeaponTag(FGameplayTag()); // [수정]

			// 3. 서버: 무기 장착 태그 제거
			ASC->RemoveLooseGameplayTag(CMGameplayTags::Player_Ability_EquipWeapon);
		}
	}
}