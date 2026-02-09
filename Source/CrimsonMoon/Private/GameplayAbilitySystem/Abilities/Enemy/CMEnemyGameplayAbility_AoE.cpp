// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGameplayAbility_AoE.h"

#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "Components/Combat/CMAoEDamageComponent.h"
#include "GameFramework/Character.h"
#include "GameplayTags/CMGameplayTags_Enemy.h"
#include "GameplayTags/CMGameplayTags_Shared.h"

UCMEnemyGameplayAbility_AoE::UCMEnemyGameplayAbility_AoE()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer Tags;
	Tags.AddTag(CMGameplayTags::Enemy_Ability_Attack);
	SetAssetTags(Tags);
}

void UCMEnemyGameplayAbility_AoE::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACMEnemyCharacterBase* EnemyCharacter = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo());
	if (!EnemyCharacter || !AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 타겟 찾기
	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());

	// TriggerData에 없다면 블랙보드 확인
	if (!TargetActor)
	{
		TargetActor = EnemyCharacter->StateTreeTargetActor;
	}

	// 모션 워핑 설정 (타겟이 있다면 바라보게 설정)
	if (TargetActor)
	{
		if (UMotionWarpingComponent* MotionWarping = EnemyCharacter->FindComponentByClass<UMotionWarpingComponent>())
		{
			MotionWarping->AddOrUpdateWarpTargetFromComponent(
			WarpTargetName,
			TargetActor->GetRootComponent(),
			NAME_None,
			true,
			FVector::ZeroVector
			);
		}
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, CMGameplayTags::Shared_Event_Attack
		);
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAoETriggered);
	WaitEventTask->ReadyForActivation();
}

void UCMEnemyGameplayAbility_AoE::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMEnemyGameplayAbility_AoE::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMEnemyGameplayAbility_AoE::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMEnemyGameplayAbility_AoE::OnAoETriggered(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return;
	}

	// AOE 컴포넌트 확인
	UCMAoEDamageComponent* AoEComp = Character->FindComponentByClass<UCMAoEDamageComponent>();
	if (AoEComp)
	{
		FVector BasisLocation = Character->GetActorLocation();
		FRotator BasisRotation = Character->GetActorRotation();

		// 소켓이 지정되어 있고, 메시에 해당 소켓이 있다면 기준점 변경
		if (AoESocketName != NAME_None && Character->GetMesh())
		{
			if (Character->GetMesh()->DoesSocketExist(AoESocketName))
			{
				BasisLocation = Character->GetMesh()->GetSocketLocation(AoESocketName);
				BasisRotation = Character->GetMesh()->GetSocketRotation(AoESocketName);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
					TEXT("지정된 소켓 '%s'가 메쉬 '%s'에 존재하지 않습니다."),
					*AoESocketName.ToString(),
					*Character->GetMesh()->GetName());
			}
		}

		// 오프셋 적용
		FVector FinalLocation = BasisLocation;
		if (!AoELocationOffset.IsZero())
		{
			FinalLocation += BasisRotation.RotateVector(AoELocationOffset);
		}

		FHitResult HitInfo;
		HitInfo.Location = FinalLocation;
		HitInfo.ImpactPoint = FinalLocation;

		AoEComp->ApplyAoEDamage(
			HitInfo,
			Character,
			GetAbilitySystemComponentFromActorInfo(),
			DamageEffectClass,
			BaseDamage,
			AttackRadius
			);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Ability '%s': Actor '%s'에 AoE Component가 존재하지 않습니다."),
			*GetNameSafe(this),
			*GetNameSafe(Character));
	}
}