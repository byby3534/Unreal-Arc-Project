// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Actors/CMExplosiveProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayCueFunctionLibrary.h"
#include "CMGameplayTags.h"

ACMExplosiveProjectile::ACMExplosiveProjectile()
{
    if (ProjectileMovement)
    {
    	ProjectileMovement->bShouldBounce = true;
    	ProjectileMovement->Bounciness = 0.3f;
    	ProjectileMovement->Friction = 0.6f;
    }

	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void ACMExplosiveProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 타이머를 돌려서 폭발
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			ExplosionTimerHandle, 
			this, 
			&ACMExplosiveProjectile::Explode, 
			FuseTime, 
			false
		);
	}
}

void ACMExplosiveProjectile::OnProjectileHit(AActor* HitActor)
{
	// 여기서는 아무것도 하지 않도록 오버라이드하여 즉시 폭발을 방지
}

void ACMExplosiveProjectile::Explode()
{
    // 범위 내의 모든 액터 찾기
    TArray<FHitResult> OutHits;
    FVector Location = GetActorLocation();
    FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);

	// Pawn만 감지하도록 설정
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		Location,
		Location,
		FQuat::Identity,
		ECC_Pawn,
		Sphere
	);

	// 감지된 액터들에게 GAS 이펙트(데미지) 적용
	if (HasAuthority() && ProjectileEffectSpec.IsValid())
	{
		TSet<AActor*> HitActors;

		for (const FHitResult& Hit : OutHits)
		{
			AActor* TargetActor = Hit.GetActor();

			// 유효한 대상이고, 아직 처리하지 않았으며, 자기 자신이나 시전자가 아닌 경우
			if (TargetActor && !HitActors.Contains(TargetActor) && TargetActor != this)
			{
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
				if (TargetASC)
				{
					// 스펙 적용
					TargetASC->ApplyGameplayEffectSpecToSelf(*ProjectileEffectSpec.Data.Get());
				}

				// 처리 목록에 추가
				HitActors.Add(TargetActor);
			}
		}
	}

	// GameplayCue 발동
	FGameplayCueParameters CueParams;
	CueParams.Location = Location;
	CueParams.Normal = GetActorUpVector();
	CueParams.Instigator = GetInstigator();
	CueParams.EffectCauser = this;
	
	// 시전자의 ASC를 가져와서 큐를 실행
	if (UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator()))
	{
		OwnerASC->ExecuteGameplayCue(CMGameplayTags::GameplayCue_Item_Throw_Explode, CueParams);
	}
	else
	{
		UGameplayCueFunctionLibrary::ExecuteGameplayCueOnActor(this, CMGameplayTags::GameplayCue_Item_Throw_Explode, CueParams);
	}

	// 폭발 후 액터 제거
	if (HasAuthority())
	{
		Destroy();
	}
}