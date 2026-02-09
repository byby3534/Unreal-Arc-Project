// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Actors/Place/CMThrowActor_MolotovProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "Items/Actors/CMZoneEffectActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayCueFunctionLibrary.h"
#include "CMGameplayTags.h"

ACMThrowActor_MolotovProjectile::ACMThrowActor_MolotovProjectile()
{
	// 화염병은 닿자마자 깨져야 하므로 튕기지 않게 설정
	if (ProjectileMovement)
	{
		ProjectileMovement->bShouldBounce = false;
		ProjectileMovement->Friction = 0.0f;
		ProjectileMovement->bSweepCollision = true; // 빠른 속도에서도 충돌 감지
	}

	// 충돌 설정 변경
	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 물리 충돌 확실하게 켜기
		SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		SphereComponent->SetNotifyRigidBodyCollision(true);
		SphereComponent->SetGenerateOverlapEvents(true);
	}
}

void ACMThrowActor_MolotovProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Hit 이벤트 바인딩
	if (HasAuthority() && SphereComponent)
	{
		SphereComponent->OnComponentHit.AddDynamic(this, &ACMThrowActor_MolotovProjectile::OnGroundHit);
	}
}

void ACMThrowActor_MolotovProjectile::OnProjectileHit(AActor* HitActor)
{
	// 서버에서만 로직 수행
	if (HasAuthority())
	{
		// 폰은 무시
		if (HitActor && HitActor->IsA(APawn::StaticClass()))
		{
			return;
		}

		FVector ImpactLocation = GetActorLocation();
        FRotator ZoneRotation = FRotator::ZeroRotator;

        // 바닥 감지
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		
		if (HitActor)
		{
			Params.AddIgnoredActor(HitActor);
		}

        bool bHitGround = GetWorld()->LineTraceSingleByChannel(
            HitResult, 
            ImpactLocation, 
            ImpactLocation - FVector(0, 0, 200.0f),
            ECC_WorldStatic, 
            Params
        );

        if (bHitGround)
        {
            ImpactLocation = HitResult.Location;
            ZoneRotation = FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).ToQuat().Rotator();
        }

        // 화염 장판(Zone) 스폰
        if (FireZoneClass)
        {
        	// Instigator가 없으면 Owner를 사용 (안전장치)
        	AActor* SpawnerInstigator = GetInstigator();
        	if (!SpawnerInstigator)
        	{
        		SpawnerInstigator = GetOwner();
        	}
        	
            ACMZoneEffectActor* FireZone = GetWorld()->SpawnActorDeferred<ACMZoneEffectActor>(
                FireZoneClass,
                FTransform(ZoneRotation, ImpactLocation),
                GetOwner(),
                Cast<APawn>(SpawnerInstigator), // Instigator 명시적 전달
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn
            );

            if (FireZone)
            {
                FireZone->InitializeDuration(FireDuration);
            	// 투척체에서 설정된 GE를 장판에 전달
            	if (ZoneEffectClass)
            	{
            		FireZone->SetZoneGameplayEffect(ZoneEffectClass);
            	}
                UGameplayStatics::FinishSpawningActor(FireZone, FTransform(ZoneRotation, ImpactLocation));
            }
        }

        // GameplayCue 발동
        FGameplayCueParameters CueParams;
        CueParams.Location = GetActorLocation();
        CueParams.Normal = bHitGround ? HitResult.ImpactNormal : FVector::UpVector;
        CueParams.Instigator = GetInstigator();

        // 시전자의 ASC를 통해 Cue 전파 
        if (UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator()))
        {
            OwnerASC->ExecuteGameplayCue(CMGameplayTags::GameplayCue_Item_Throw_Fire, CueParams);
        }
        else
        {
            UGameplayCueFunctionLibrary::ExecuteGameplayCueOnActor(this, CMGameplayTags::GameplayCue_Item_Throw_Fire, CueParams);
        }

        // 투척체 파괴
        Destroy();
	}
}

void ACMThrowActor_MolotovProjectile::OnGroundHit(
	UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 기존 로직 함수를 호출
	OnProjectileHit(OtherActor);
}
