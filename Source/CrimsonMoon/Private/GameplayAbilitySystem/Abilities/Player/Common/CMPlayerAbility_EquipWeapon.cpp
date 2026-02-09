// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_EquipWeapon.h"
#include "CMGameplayTags.h"
#include "Character/CMCharacterBase.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "DataAssets/Weapon/CMDataAsset_WeaponData.h"
#include "Items/Weapons/CMPlayerWeapon.h"
#include "Items/Weapons/CMWeaponBase.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Structs/CMStructTypes.h"
#include "Components/UI/UPawnUIComponent.h"

UCMPlayerAbility_EquipWeapon::UCMPlayerAbility_EquipWeapon()
{
	// --- 어빌리티 태그 설정 ---
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_EquipWeapon); // 예시 태그. 필요시 수정
	SetAssetTags(TagsToAdd);

	ActivationOwnedTags.AddTag(CMGameplayTags::Player_Status_Equipping);   // 이 어빌리티가 활성화되어 있는 동안 소유자(캐릭터)에게 이 태그를 부여
	ActivationBlockedTags.AddTag(CMGameplayTags::Player_Status_Equipping); // Player_Status_Equipping 태그가 있다면 이 어빌리티는 활성화될 수 없습니다. (중복 실행 방지)

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;

	EquipEventTag = CMGameplayTags::Player_Event_EquipWeapon;
}

bool UCMPlayerAbility_EquipWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
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

	// 무기를 들고 있지 않아야 활성화 가능
	if (CombatComponent->GetCharacterCurrentEquippedWeapon() != nullptr)
	{
		return false;
	}

	// 캐릭터 클래스 검증: 서버에서만 검증 (클라이언트는 ASC 태그가 아직 리플리케이트되지 않았을 수 있음)
	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		const ACMPlayerWeapon* WeaponToCheck = Cast<ACMPlayerWeapon>(CombatComponent->GetCharacterCarriedWeaponByTag(WeaponToEquipTag));
		if (WeaponToCheck && WeaponToCheck->WeaponData)
		{
			const FGameplayTag RequiredCharacterClassTag = WeaponToCheck->WeaponData->RequiredCharacterClassTag;

			// 무기에 클래스 태그가 설정되어 있지 않으면 장착 불가 (모든 무기는 클래스를 명시해야 함)
			if (!RequiredCharacterClassTag.IsValid())
			{
				return false;
			}

			// 클래스 태그가 있으면 캐릭터가 해당 태그를 가지고 있는지 검증
			const UCMAbilitySystemComponent* ASC = OwnerCharacter->GetCMAbilitySystemComponent();
			if (!ASC || !ASC->HasMatchingGameplayTag(RequiredCharacterClassTag))
			{
				// 캐릭터가 요구 클래스 태그를 가지고 있지 않음
				return false;
			}
		}
	}

	return true;
}


void UCMPlayerAbility_EquipWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 몽타주가 없으면, 로직만 즉시 실행하고 어빌리티를 종료
	if (!EquipMontage)
	{
		HandleEquipLogic();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 몽타주 재생 태스크 생성
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, EquipMontage);
	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);

	PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);

	//게임플레이 이벤트 수신 대기 태스크 생성
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EquipEventTag);
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);

	// --- 3. 두 태스크 모두 활성화 ---
	PlayMontageTask->ReadyForActivation();
	WaitEventTask->ReadyForActivation();
}

void UCMPlayerAbility_EquipWeapon::OnMontageEnded()
{
	// HandleEquipLogic();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_EquipWeapon::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMPlayerAbility_EquipWeapon::OnEventReceived(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		HandleEquipLogic();
	}
}

void UCMPlayerAbility_EquipWeapon::HandleEquipLogic()
{
	// 해당 로직은 서버에서만 실행
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	ACMCharacterBase* OwnerCharacter = GetCMCharacterFromActorInfo();
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

	if (ACMPlayerWeapon* PlayerWeapon = Cast<ACMPlayerWeapon>(PawnCombatComponent->GetCharacterCarriedWeaponByTag(WeaponToEquipTag)))
	{
		// 데이터 에셋 포인터를 가져오고 nullptr 체크
		if (const UCMDataAsset_WeaponData* WeaponData = PlayerWeapon->WeaponData)
		{
			// 1. 서버: 능력 부여
			TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;

			// Reserve를 사용해서 메모리 최적화
			GrantedAbilitySpecHandles.Reserve(WeaponData->DefaultWeaponAbilities.Num());

			// UPawnUIComponent 가져오기 (서버에서만 실행)
			UPawnUIComponent* UIComponent = OwnerCharacter->FindComponentByClass<UPawnUIComponent>();

			for (const FCMPlayerAbilitySet& AbilitySet : WeaponData->DefaultWeaponAbilities)
			{
				// 스펙 생성
				FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
				AbilitySpec.SourceObject = PlayerWeapon;
				AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);

				const FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(AbilitySpec);

				// 생성된 무기 스펙 부여
				GrantedAbilitySpecHandles.Add(SpecHandle);

				// 서버: UI 컴포넌트에 아이콘 추가 (리플리케이트됨)
				if (UIComponent && !AbilitySet.SoftAbilityIconMaterial.IsNull())
				{
					UIComponent->AddAbilityIcon(AbilitySet);
				}
			}
			PlayerWeapon->AssignGrantedAbilitySpecHandles(GrantedAbilitySpecHandles);

			// 2. 서버: 상태 변경 (OnRep 호출)
			PawnCombatComponent->SetCurrentEquippedWeaponTag(WeaponToEquipTag); // [수정]

			// 3. 서버: 무기 장착 태그 추가
			ASC->AddLooseGameplayTag(CMGameplayTags::Player_Ability_EquipWeapon);
		}
	}
}