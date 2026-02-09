// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Skill/Projectile/CMProjectileActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "GameplayEffect.h"
#include "CrimsonMoon/DebugHelper.h"
#include "Components/SphereComponent.h"
#include "Components/Combat/CMAoEDamageComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "System/CMObjectPoolingManager.h"

ACMProjectileActor::ACMProjectileActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionComponent->SetSphereRadius(32.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->SetIsReplicated(true);

	bReplicates = true;

	SetReplicateMovement(true);
}

void ACMProjectileActor::SetDamageInfo(const TSubclassOf<UGameplayEffect>& InDamageEffect, const float InBaseDamage)
{
	DamageEffect = InDamageEffect;
	BaseDamage = InBaseDamage;
}

void ACMProjectileActor::SetHitGameplayCueTag(const FGameplayTag& InHitCueTag)
{
	HitGameplayCueTag = InHitCueTag;
}

void ACMProjectileActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 Hit 처리
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == GetInstigator())
	{
		return;
	}

	APawn* SourcePawn = GetInstigator<APawn>();
	APawn* TargetPawn = Cast<APawn>(OtherActor);
	if (!SourcePawn || !TargetPawn)
	{
		return;
	}

	FHitResult FinalHitResult = SweepResult;

	if (!bFromSweep)
	{
		FinalHitResult.bBlockingHit = true;
		FinalHitResult.ImpactPoint = GetActorLocation();
		FinalHitResult.ImpactNormal = GetActorForwardVector();
		FinalHitResult.HitObjectHandle = FActorInstanceHandle(OtherActor);
	}

	// AoE 컴포넌트 체크
	if (AoEComponent)
	{
		// AoE 데미지 처리
		if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourcePawn))
		{
			AoEComponent->ApplyAoEDamage(FinalHitResult, SourcePawn, SourceASC, DamageEffect, BaseDamage);
		}
	}
	else
	{
		// 단일 타겟 데미지 처리
		ProcessSingleTargetHit(OtherActor, FinalHitResult, SourcePawn, TargetPawn);
	}

	// 투사체 이동 중지
	GetProjectileMovement()->StopSimulating(FinalHitResult);
}

void ACMProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void ACMProjectileActor::InitProjectileTransform(const FTransform& SpawnTransform)
{
	if (HasAuthority())
	{
		// 투사체 이동
		// 충돌 시 StopSimulating 되어있는것 풀어주기
		ProjectileMovement->SetUpdatedComponent(RootComponent);
		GetProjectileMovement()->Velocity = SpawnTransform.GetRotation().GetForwardVector() * GetProjectileMovement()->InitialSpeed;
		GetProjectileMovement()->Activate(true);
	}
}

void ACMProjectileActor::PoolActivate_Implementation()
{
	bIsAvailablePool = false;

	SetActorHiddenInGame(false);

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACMProjectileActor::HandleBeginOverlap);

	// 나이아가라 컴포넌트 재활성화 (모든 클라이언트에서)
	if (HasAuthority())
	{
		MulticastActivateNiagara();
	}

	SetLifeSpan(LifeSpan);
}

void ACMProjectileActor::PoolReturn_Implementation()
{
	bIsAvailablePool = true;

	SetActorHiddenInGame(true);

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionComponent->OnComponentBeginOverlap.RemoveAll(this);

	// 나이아가라 컴포넌트 비활성화 (모든 클라이언트에서)
	if (HasAuthority())
	{
		MulticastDeactivateNiagara();
	}

	DamageEffect = nullptr;
	BaseDamage = 0.f;
	HitGameplayCueTag = FGameplayTag();

	SetLifeSpan(0.f);

	SetOwner(nullptr);
	SetInstigator(nullptr);
}

bool ACMProjectileActor::IsAvailable_Implementation() const
{
	return bIsAvailablePool;
}

void ACMProjectileActor::ReleaseToPool_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (UCMObjectPoolingManager* PoolingManager = World->GetSubsystem<UCMObjectPoolingManager>())
		{
			PoolingManager->ReturnObject(this);
		}
	}
}

void ACMProjectileActor::LifeSpanExpired()
{
	Execute_ReleaseToPool(this);
}

void ACMProjectileActor::ProcessSingleTargetHit(AActor* HitActor, const FHitResult& HitResult, APawn* SourcePawn, APawn* TargetPawn)
{
	if (!HitActor || !GetInstigator())
	{
		return;
	}

	if (!SourcePawn || !TargetPawn || !UCMFunctionLibrary::IsTargetPawnHostile(SourcePawn, TargetPawn))
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);

	if (SourceASC && TargetASC && DamageEffect)
	{
		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		ContextHandle.AddInstigator(GetInstigator(), GetInstigator());
		ContextHandle.AddHitResult(HitResult);

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);

		if (SpecHandle.Data.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(CMGameplayTags::Shared_SetByCaller_BaseDamage, BaseDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}

		// Hit GameplayCue 실행
		if (HitGameplayCueTag.IsValid())
		{
			FGameplayCueParameters CueParams;
			CueParams.Instigator = SourcePawn;
			CueParams.EffectCauser = SourcePawn;
			CueParams.SourceObject = this;
			CueParams.TargetAttachComponent = HitActor->GetRootComponent();
			CueParams.Location = HitResult.ImpactPoint;
			CueParams.Normal = HitResult.ImpactNormal;

			SourceASC->ExecuteGameplayCue(HitGameplayCueTag, CueParams);
		}
	}
}

void ACMProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	AoEComponent = FindComponentByClass<UCMAoEDamageComponent>();
}

void ACMProjectileActor::MulticastActivateNiagara_Implementation()
{
	TArray<UNiagaraComponent*> NiagaraComponents;
	GetComponents<UNiagaraComponent>(NiagaraComponents);
	for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
	{
		if (NiagaraComp)
		{
			NiagaraComp->Activate(true);
		}
	}
}

void ACMProjectileActor::MulticastDeactivateNiagara_Implementation()
{
	TArray<UNiagaraComponent*> NiagaraComponents;
	GetComponents<UNiagaraComponent>(NiagaraComponents);
	for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
	{
		if (NiagaraComp)
		{
			NiagaraComp->Deactivate();
		}
	}
}