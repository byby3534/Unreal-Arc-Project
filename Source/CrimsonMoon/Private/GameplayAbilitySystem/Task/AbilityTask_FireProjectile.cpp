// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Task/AbilityTask_FireProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayAbilitySystem/Abilities/CMGameplayAbility.h"
#include "GameplayAbilitySystem/Abilities/Skill/Projectile/CMProjectileActor.h"
#include "System/CMObjectPoolingManager.h"

UAbilityTask_FireProjectile* UAbilityTask_FireProjectile::FireProjectile(
	UGameplayAbility* OwningAbility,
	TSubclassOf<ACMProjectileActor> ProjectileClass,
	FTransform SpawnTransform,
	TSubclassOf<UGameplayEffect> DamageEffect,
	float BaseDamage)
{
	UAbilityTask_FireProjectile* Task = NewAbilityTask<UAbilityTask_FireProjectile>(OwningAbility);
	Task->SkillActorClass = ProjectileClass;
	Task->SpawnTransform = SpawnTransform;
	Task->DamageEffect = DamageEffect;
	Task->BaseDamage = BaseDamage;
	return Task;
}

void UAbilityTask_FireProjectile::Activate()
{
	Super::Activate();

	UCMGameplayAbility* MyAbility = Cast<UCMGameplayAbility>(Ability);
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MyAbility->GetAvatarActorFromActorInfo());

	if (!MyAbility || !SkillActorClass || !SourceASC)
	{
		if (OnFailed.IsBound())
		{
			OnFailed.Broadcast(nullptr);
		}
		EndTask();
		return;
	}

	UWorld* World = GetWorld();
	UCMObjectPoolingManager* PoolingManager = World->GetSubsystem<UCMObjectPoolingManager>();

	if (!PoolingManager)
	{
		if (OnFailed.IsBound())
		{
			OnFailed.Broadcast(nullptr);
		}
		EndTask();
		return;
	}

	if (GetAvatarActor()->HasAuthority())
	{
		// 오브젝트 풀링에서 오브젝트 획득
		ACMProjectileActor* SpawnedActor = Cast<ACMProjectileActor>(PoolingManager->GetObject(SkillActorClass));
		if (!SpawnedActor)
		{
			if (OnFailed.IsBound())
			{
				OnFailed.Broadcast(nullptr);
			}
			EndTask();
			return;
		}

		AActor* AvatarActor = MyAbility->GetAvatarActorFromActorInfo();
		SpawnedActor->SetOwner(AvatarActor);
		SpawnedActor->SetInstigator(Cast<APawn>(AvatarActor));

		SpawnedActor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::ResetPhysics);

		// 데미지 정보 설정
		SpawnedActor->SetDamageInfo(DamageEffect, BaseDamage);

		// 위치 초기화
		SpawnedActor->InitProjectileTransform(SpawnTransform);

		// 오브젝트 활성화
		PoolingManager->ActivateObject(SpawnedActor);

		if (OnSuccess.IsBound())
		{
			OnSuccess.Broadcast(SpawnedActor);
		}
	}

	EndTask();
}