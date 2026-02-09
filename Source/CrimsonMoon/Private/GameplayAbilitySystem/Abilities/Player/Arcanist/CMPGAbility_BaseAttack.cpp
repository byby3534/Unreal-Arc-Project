// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Player/Arcanist/CMPGAbility_BaseAttack.h"
#include "AbilitySystemComponent.h"
#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/CMLineTraceComponent.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GameplayAbilitySystem/Abilities/Skill/Projectile/CMProjectileActor.h"
#include "GameplayAbilitySystem/Task/AbilityTask_FireProjectile.h"

UCMPGAbility_BaseAttack::UCMPGAbility_BaseAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UCMPGAbility_BaseAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ProjectileClass.Get())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 커밋에 실패하면 (예: 스태미나 부족) 어빌리티를 즉시 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!FireProjectileMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (SoundEffectClass)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// GE Spec 생성
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SoundEffectClass, GetAbilityLevel(), ContextHandle);

			if (SpecHandle.IsValid())
			{
				// GE 적용 및 핸들 저장 (EndAbility 시 초기화)
				SoundEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	// 몽타주 재생 태스크 생성
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FireProjectileMontage);
	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	//PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	PlayMontageTask->ReadyForActivation();

	// Event Tag 대기 태스크 생성
	UAbilityTask_WaitGameplayEvent* WaitEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			CMGameplayTags::Player_Event_FireProjectile
			);
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnFireProjectileEvent);
	WaitEventTask->ReadyForActivation();

}

void UCMPGAbility_BaseAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (SoundEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// 이펙트 제거
			ASC->RemoveActiveGameplayEffect(SoundEffectHandle);
		}

		// 핸들 초기화
		SoundEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UCMPGAbility_BaseAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UCMPGAbility_BaseAttack::OnMontageCompleted()
{
	// 투사체 발사 후 어빌리티 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPGAbility_BaseAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPGAbility_BaseAttack::OnFireProjectileEvent(FGameplayEventData Payload)
{
	// 서버에서만 투사체 발사
	if (GetAvatarActorFromActorInfo()->HasAuthority())
	{
		FireProjectile();
	}
}

void UCMPGAbility_BaseAttack::FireProjectile()
{
	ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();
	if (!OwnerCharacter)
	{
		return;
	}

	UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();
	if (!PlayerCombatComponent)
	{
		return;
	}

	const float BaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel());

	// 이 함수는 서버에서만 호출됨
	const FTransform SpawnTransform = CalculateProjectileSpawnTransform();
	UAbilityTask_FireProjectile* Task = UAbilityTask_FireProjectile::FireProjectile(
		this,
		ProjectileClass,
		SpawnTransform,
		DamageEffect,
		BaseDamage * SkillDamageMultiplier
		);
	Task->ReadyForActivation();
}

FTransform UCMPGAbility_BaseAttack::CalculateProjectileSpawnTransform()
{
	// 1. 발사 시작 위치 계산
	FTransform FireTransform;
	ACMPlayerCharacterBase* MyCharacter = GetCMPlayerCharacterFromActorInfo();

	USkeletalMeshComponent* MyMesh = UCMFunctionLibrary::FindSkeletalMeshByTag(MyCharacter, ComponentTag);
	if (MyCharacter && MyMesh && SpawnSocketName != NAME_None)
	{
		FireTransform = MyMesh->GetSocketTransform(SpawnSocketName);
	}
	else
	{
		FireTransform = GetAvatarActorFromActorInfo()->GetActorTransform();
	}
	FireTransform = SpawnTransformOffset * FireTransform;
	FVector SpawnLocation = FireTransform.GetLocation();

	// 2. 조준점 위치 계산
	FVector HitLocation;

	// 먼저 TargetLock이 활성화되어 있고 잠긴 타겟이 있는지 확인
	UPlayerCombatComponent* CombatComp = MyCharacter->GetPawnCombatComponent();
	AActor* LockedTarget = CombatComp ? CombatComp->CurrentLockOnTarget.Get() : nullptr;

	if (LockedTarget)
	{
		// 잠긴 타겟이 있으면 타겟의 위치를 조준점으로 사용
		HitLocation = LockedTarget->GetActorLocation();
	}
	else
	{
		// 잠긴 타겟이 없으면 기존 카메라 라인 트레이스 사용
		UCMLineTraceComponent* LineTraceComponent = MyCharacter->FindComponentByClass<UCMLineTraceComponent>();

		if (LineTraceComponent)
		{
			FHitResult HitResult;
			if (LineTraceComponent->GetAimHitResultFromLineTrace(HitResult, LineTraceRange))
			{
				HitLocation = HitResult.Location;
			}
			else
			{
				HitLocation = HitResult.TraceEnd;
			}
		}
		else
		{
			HitLocation = SpawnLocation + (GetAvatarActorFromActorInfo()->GetActorForwardVector() * LineTraceRange);
		}
	}

	// 3. 최종 발사 트랜스폼 계산
	FRotator FinalRotation = (HitLocation - SpawnLocation).Rotation();
	return FTransform(FinalRotation, SpawnLocation);
}