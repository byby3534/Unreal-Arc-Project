// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Common/CMAbility_SpawnAndRegisterWeapon.h"
#include "Character/CMCharacterBase.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Engine/World.h"
#include "Items/Weapons/CMWeaponBase.h"

UCMAbility_SpawnAndRegisterWeapon::UCMAbility_SpawnAndRegisterWeapon()
{
	// 이 어빌리티는 서버에서만 실행되고 즉시 종료
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UCMAbility_SpawnAndRegisterWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!HasAuthority(&ActivationInfo))
	{
		return;
	}

	// 1. 유효성 검사 (서버에서만 실행)
	ACMCharacterBase* OwnerCharacter = GetCMCharacterFromActorInfo();
	if (!OwnerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UPawnCombatComponent* CombatComponent = OwnerCharacter->GetPawnCombatComponent();
	if (!CombatComponent || !WeaponClassToSpawn || !WeaponTagToRegister.IsValid() || InitialSocketName == NAME_None)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1.5. 이미 등록된 무기인지 확인
	if (CombatComponent->GetCharacterCarriedWeaponByTag(WeaponTagToRegister))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false); // 이미 있으므로 취소
		return;
	}

	// 2. 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	SpawnParams.Instigator = OwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 3. 무기 스폰 (공용 로직)
	if (ACMWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<ACMWeaponBase>(WeaponClassToSpawn, SpawnParams))
	{
		// 4. [중요] 스폰된 무기를 PawnCombatComponent에 등록
		// bRegisterAsEquippedWeapon 값에 따라 즉시 장착 또는 등록만 처리
		CombatComponent->RegisterSpawnedWeapon(WeaponTagToRegister, SpawnedWeapon, bRegisterAsEquippedWeapon);

		// 5. 초기 소켓(등)에 부착 (공용 로직)
		SpawnedWeapon->AttachToComponent(
			OwnerCharacter->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			InitialSocketName
			);

		// 6. 무기 숨기기 (장착 전까지 보이지 않도록)
		if (!bRegisterAsEquippedWeapon)
		{
			SpawnedWeapon->HideWeapon();
		}
	}

	// 7. 어빌리티 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UCMAbility_SpawnAndRegisterWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->IsNetAuthority())
	{
		return false;
	}

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}