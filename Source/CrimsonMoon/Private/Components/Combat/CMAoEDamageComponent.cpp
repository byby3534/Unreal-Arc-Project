// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Combat/CMAoEDamageComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "CrimsonMoon/DebugHelper.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

UCMAoEDamageComponent::UCMAoEDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCMAoEDamageComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCMAoEDamageComponent::ApplyAoEDamage(const FHitResult& HitResult, AActor* InstigatorActor, UAbilitySystemComponent* SourceASC, TSubclassOf<UGameplayEffect> InDamageEffect, float InBaseDamage, float AOERadius /*= 0*/)
{
	if (!GetWorld() || !SourceASC || !InDamageEffect)
	{
		Debug::Print("AoE Damage", "Invalid parameters for AoE damage", FColor::Red);
		return;
	}

	// 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	APawn* SourcePawn = Cast<APawn>(InstigatorActor);
	if (!SourcePawn)
	{
		return;
	}

	float FinalExplosionRadius = ExplosionRadius;
	if (AOERadius > 0)
	{
		FinalExplosionRadius = AOERadius;
	}

	// 반경 내 액터 수집
	TArray<AActor*> HitActors;
	GetActorsInRadius(HitResult.ImpactPoint, HitActors, FinalExplosionRadius);

	// 디버그 드로잉
	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, FinalExplosionRadius, 32, FColor::Red, false, DebugDrawDuration, 0, 2.0f);
	}

	// 각 액터에 데미지 적용
	for (AActor* HitActor : HitActors)
	{
		if (!IsValidTarget(HitActor, InstigatorActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC)
		{
			continue;
		}

		APawn* TargetPawn = Cast<APawn>(HitActor);
		if (!TargetPawn)
		{
			return;
		}

		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(GetOwner());
		ContextHandle.AddInstigator(InstigatorActor, InstigatorActor);

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(InDamageEffect, 1.0f, ContextHandle);

		if (SpecHandle.Data.IsValid() && UCMFunctionLibrary::IsTargetPawnHostile(SourcePawn, TargetPawn))
		{
			SpecHandle.Data->SetSetByCallerMagnitude(CMGameplayTags::Shared_SetByCaller_BaseDamage, InBaseDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
}

void UCMAoEDamageComponent::GetActorsInRadius(FVector Location, TArray<AActor*>& OutActors, float AOERadius /*= 0*/)
{
	if (!GetWorld())
	{
		return;
	}

	float FinalExplosionRadius = ExplosionRadius;
	if (AOERadius > 0)
	{
		FinalExplosionRadius = AOERadius;
	}

	OutActors.Empty();

	// 반경 내 액터 수집
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner()); // 투사체 자체 무시

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Location,
		FinalExplosionRadius,
		ObjectTypes,
		APawn::StaticClass(), // Pawn만 검사
		ActorsToIgnore,
		OutActors
		);
}

bool UCMAoEDamageComponent::IsValidTarget(AActor* Actor, AActor* InstigatorActor) const
{
	if (!Actor)
	{
		return false;
	}

	// 발사자 무시 옵션이 켜져 있으면 발사자는 제외
	if (bIgnoreInstigator && Actor == InstigatorActor)
	{
		return false;
	}

	// Pawn이 아니면 제외
	if (!Actor->IsA<APawn>())
	{
		return false;
	}

	// AbilitySystemComponent가 없으면 제외
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (!TargetASC)
	{
		return false;
	}

	return true;
}