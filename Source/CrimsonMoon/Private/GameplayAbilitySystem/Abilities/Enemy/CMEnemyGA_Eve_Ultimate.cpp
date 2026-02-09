// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Enemy/CMEnemyGA_Eve_Ultimate.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "GameplayTags/CMGameplayTags_Enemy.h"
#include "GameplayTags/CMGameplayTags_Shared.h"
#include "Kismet/GameplayStatics.h"

UCMEnemyGA_Eve_Ultimate::UCMEnemyGA_Eve_Ultimate()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer Tags;
	Tags.AddTag(CMGameplayTags::Enemy_Ability_Attack_Ultimate);
	SetAssetTags(Tags);

	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_HitReact);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Groggy);
}

void UCMEnemyGA_Eve_Ultimate::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACMEnemyCharacterBase* BossCharacter = Cast<ACMEnemyCharacterBase>(ActorInfo->AvatarActor.Get());
	if (!BossCharacter || !UltimateMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 맵 중앙 찾기 및 순간이동
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), ArenaCenterActorTag, FoundActors);

	if (FoundActors.Num() > 0 && FoundActors[0])
	{
		BossCharacter->TeleportTo(FoundActors[0]->GetActorLocation(), FRotator::ZeroRotator);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to find ArenaCenter Actor with tag: %s"), *GetName(), *ArenaCenterActorTag.ToString());
		// 중앙을 못 찾아도 현재 위치에서 시전
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, UltimateMontage
		);

	MontageTask->OnCompleted.AddDynamic(this, &UCMEnemyGA_Eve_Ultimate::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &UCMEnemyGA_Eve_Ultimate::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UCMEnemyGA_Eve_Ultimate::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UCMEnemyGA_Eve_Ultimate::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// 3. [수정됨] 경고 장판 GameplayCue 실행
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && WarningCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = BossCharacter->GetActorLocation();
		CueParams.Instigator = BossCharacter;
		ASC->ExecuteGameplayCue(WarningCueTag, CueParams);
	}

	// 4. 폭발 이벤트 대기
	if (ExplosionEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, ExplosionEventTag, nullptr, false, false
			);
		WaitEventTask->EventReceived.AddDynamic(this, &UCMEnemyGA_Eve_Ultimate::OnExplosionTriggered);
		WaitEventTask->ReadyForActivation();
	}
}

void UCMEnemyGA_Eve_Ultimate::OnExplosionTriggered(FGameplayEventData Payload)
{
	ACMEnemyCharacterBase* BossCharacter = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo());
	if (!BossCharacter)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && ExplosionCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = BossCharacter->GetActorLocation();
		CueParams.Instigator = BossCharacter;
		ASC->ExecuteGameplayCue(ExplosionCueTag, CueParams);
	}

	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACMPlayerCharacterBase::StaticClass(), AllPlayers);

	const FVector BossLocation = BossCharacter->GetActorLocation();

	for (AActor* PlayerActor : AllPlayers)
	{
		if (!PlayerActor)
		{
			continue;
		}

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(BossCharacter);
		QueryParams.AddIgnoredActor(PlayerActor);

		const FVector StartTrace = BossLocation + FVector(0, 0, 50.0f);
		const FVector EndTrace = PlayerActor->GetActorLocation() + FVector(0, 0, 50.0f);

		bool bHitWall = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, QueryParams);

		// 벽에 막히지 않았다면 데미지 적용
		if (!bHitWall)
		{
			if (DamageEffectClass)
			{
				FGameplayEffectSpecHandle SpecHandle = MakeDamageEffectSpecHandle(
					DamageEffectClass,
					SkillBaseDamage,
					0.0f,
					FGameplayTag(),
					0
					);

				if (SpecHandle.IsValid())
				{
					if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerActor))
					{
						ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[%s] DamageEffectClass is missing!"), *GetName());
			}
		}
	}
}

void UCMEnemyGA_Eve_Ultimate::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMEnemyGA_Eve_Ultimate::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}