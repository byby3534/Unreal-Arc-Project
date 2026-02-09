// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Actors/CMThrowProjectileActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

// Sets default values
ACMThrowProjectileActor::ACMThrowProjectileActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	// 충돌 컴포넌트
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(20.0f);

	// 콜리전 설정 강화
	SphereComponent->SetCollisionProfileName(TEXT("Projectile"));
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); 
	
	// 메쉬 컴포넌트
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 발사체 움직임 컴포넌트
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(SphereComponent);

	// 기본값
	ProjectileMovement->InitialSpeed = 1000.0f; 
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	
	// 튕김 설정
	ProjectileMovement->Bounciness = 0.3f; 
	ProjectileMovement->Friction = 0.2f;   
}

void ACMThrowProjectileActor::SetProjectileMesh(UStaticMesh* NewMesh)
{
	if (MeshComponent && NewMesh)
	{
		MeshComponent->SetStaticMesh(NewMesh);
	}
}

void ACMThrowProjectileActor::InitializeProjectile(const FCMThrowableData& ThrowableData)
{
	if (ProjectileMovement)
	{
		// 데이터 에셋 값 적용
		ProjectileMovement->InitialSpeed = ThrowableData.ThrowSpeed;
		ProjectileMovement->MaxSpeed = ThrowableData.ThrowSpeed;
		ProjectileMovement->ProjectileGravityScale = ThrowableData.GravityScale;
		
		// 현재 속도 갱신
		if (GetVelocity().SizeSquared() > 0.f)
		{
			ProjectileMovement->Velocity = GetVelocity().GetSafeNormal() * ThrowableData.ThrowSpeed;
		}
	}
}

void ACMThrowProjectileActor::InitFakeProjectile()
{
	bIsFake = true;
	
	if (SphereComponent)
	{
		SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	}
	
	bReplicates = false;
}

void ACMThrowProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 충돌 이벤트를 처리
	if (HasAuthority())
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ACMThrowProjectileActor::OnOverlapBegin);

		// 아무데도 안 맞고 영원히 날아가면 5초 뒤 삭제
		SetLifeSpan(5.0f);
	}
}

void ACMThrowProjectileActor::OnRep_Instigator()
{
	Super::OnRep_Instigator();
}

void ACMThrowProjectileActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetInstigator() || OtherActor == this)
	{
		return;
	}

	OnProjectileHit(OtherActor);
}

void ACMThrowProjectileActor::OnProjectileHit(AActor* HitActor)
{
	// 기본 동작: 단일 대상에게 이펙트 적용 후 파괴
	if (HasAuthority() && ProjectileEffectSpec.IsValid())
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (TargetASC)
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*ProjectileEffectSpec.Data.Get());
		}
		Destroy();
	}
}
