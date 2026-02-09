// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGA_RangedAttack.h"

#include "CMGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "GameplayAbilitySystem/Abilities/Skill/Projectile/CMProjectileActor.h"
#include "GameplayAbilitySystem/Task/AbilityTask_FireProjectile.h"
#include "Kismet/KismetMathLibrary.h"

UCMEnemyGA_RangedAttack::UCMEnemyGA_RangedAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer Tags;
	Tags.AddTag(CMGameplayTags::Enemy_Ability_Attack_Ranged);
	SetAssetTags(Tags);
}

void UCMEnemyGA_RangedAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!AttackMontage || !ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RangedAttack: 몽타주 또는 ProjectileClass가 설정되지 않았습니다!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// 발사 이벤트 대기
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, CMGameplayTags::Player_Event_FireProjectile
		);
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnFireProjectileEvent);
	WaitEventTask->ReadyForActivation();
}

void UCMEnemyGA_RangedAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMEnemyGA_RangedAttack::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMEnemyGA_RangedAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMEnemyGA_RangedAttack::OnFireProjectileEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	ACMEnemyCharacterBase* OwnerCharacter = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter)
	{
		return;
	}

	// 발사 위치(소켓) 계산
	const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	const FVector SpawnLocation = (Mesh && MuzzleSocketName != NAME_None) ? Mesh->GetSocketLocation(MuzzleSocketName) : OwnerCharacter->GetActorLocation();

	// 발사 각도 계산
	const FRotator SpawnRotation = GetRotationToTarget(SpawnLocation);
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	// 투사체 발사 태스크 실행
	UAbilityTask_FireProjectile* FireTask = UAbilityTask_FireProjectile::FireProjectile(
		this,
		ProjectileClass,
		SpawnTransform,
		DamageEffectClass,
		BaseDamage
		);

	FireTask->ReadyForActivation();
}

FRotator UCMEnemyGA_RangedAttack::GetRotationToTarget(const FVector& StartLocation) const
{
	ACMEnemyCharacterBase* OwnerCharacter = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo());

	// AI가 인식하고 있는 StateTree 타겟 가져오기
	AActor* TargetActor = OwnerCharacter ? OwnerCharacter->StateTreeTargetActor : nullptr;

	if (TargetActor)
	{
		return UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetActor->GetActorLocation());
	}

	// 타겟이 없으면 그냥 정면 발사
	return GetAvatarActorFromActorInfo()->GetActorRotation();
}