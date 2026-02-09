// Copyright CrimsonMoon Team. All Rights Reserved.

#include "GameplayAbilitySystem/Abilities/Skill/CMAdventActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

ACMAdventActor::ACMAdventActor()
{
	// ProjectileMovement 비활성화 (장판은 고정)
	if (ProjectileMovement)
	{
		ProjectileMovement->bAutoActivate = false;
		ProjectileMovement->SetActive(false);
	}
}

void ACMAdventActor::InitProjectileTransform(const FTransform& SpawnTransform)
{
	// ProjectileMovement를 시작하지 않음 (장판은 고정)
	// 위치만 설정됨 (FireProjectile 태스크에서 이미 SetActorTransform 호출)
}

void ACMAdventActor::SetDotDamageInfo(const TSubclassOf<UGameplayEffect>& InDotDamageEffect,
	float InDotDamageMultiplier,
	float InDotInterval,
	float InDotDuration,
	float InExplosionRadius)
{
	DotDamageEffect = InDotDamageEffect;
	DotDamageMultiplier = InDotDamageMultiplier;
	DotInterval = InDotInterval;
	DotDuration = InDotDuration;
	ExplosionRadius = InExplosionRadius;

	// LifeSpan 설정 (DotDuration 후 자동 제거)
	SetLifeSpan(DotDuration);
}

void ACMAdventActor::StartDotTimer()
{
	// 도트 타이머 시작
	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FTimerDelegate DotDelegate;
			DotDelegate.BindUObject(this, &ThisClass::OnDotTick);

			World->GetTimerManager().SetTimer(
				DotTimerHandle,
				DotDelegate,
				DotInterval,
				true,       // Loop
				DotInterval // 첫 실행은 DotInterval 후
				);
		}
	}
}

void ACMAdventActor::SetDebugDrawSettings(bool bEnable, float Duration)
{
	bEnableDebugDraw = bEnable;
	DebugDrawDuration = Duration;
}

void ACMAdventActor::PoolActivate_Implementation()
{
	Super::PoolActivate_Implementation();
}

void ACMAdventActor::PoolReturn_Implementation()
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DotTimerHandle);
	}

	// 도트 정보 초기화
	DotDamageEffect = nullptr;
	DotDamageMultiplier = 0.5f;
	DotInterval = 0.5f;
	DotDuration = 5.0f;
	ExplosionRadius = 500.0f;
	bEnableDebugDraw = false;
	DebugDrawDuration = 3.0f;

	Super::PoolReturn_Implementation();
}

void ACMAdventActor::OnDotTick()
{
	// 도트 데미지 적용
	ApplyDotDamage();
}

void ACMAdventActor::ApplyDotDamage()
{
	APawn* SourcePawn = GetInstigator();
	if (!SourcePawn)
	{
		return;
	}

	ACMPlayerCharacterBase* OwnerCharacter = Cast<ACMPlayerCharacterBase>(SourcePawn);
	if (!OwnerCharacter)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourcePawn);
	if (!SourceASC)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector Location = GetActorLocation();

	// SphereOverlap으로 범위 내 적 탐색
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(SourcePawn);
	TArray<AActor*> OverlappedActors;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		World,
		Location,
		ExplosionRadius,
		ObjectTypes,
		nullptr,
		IgnoredActors,
		OverlappedActors
		);

	// 도트 데미지 장판 범위 디버그 드로우 (노란색)
	if (bEnableDebugDraw)
	{
		DrawDebugSphere(
			World,
			Location,
			ExplosionRadius,
			32,
			FColor::Yellow,
			false,
			DebugDrawDuration,
			0,
			1.5f
			);
	}

	if (bHit)
	{
		for (AActor* HitActor : OverlappedActors)
		{
			if (!HitActor)
			{
				continue;
			}

			// 타겟의 ASC 가져오기
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
			if (!TargetASC)
			{
				continue;
			}

			// 팀 체크
			APawn* TargetPawn = Cast<APawn>(HitActor);
			if (UCMFunctionLibrary::IsTargetPawnHostile(SourcePawn, TargetPawn))
			{
				// 도트 데미지 적용
				if (DotDamageEffect)
				{
					UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();

					// BaseDamage는 부모 클래스에서 설정됨
					const float FinalDamage = BaseDamage * DotDamageMultiplier;
					const float BaseGroggyDamage = PlayerCombatComponent->GetPlayerCurrentWeaponHeavyGroggyDamage(1);

					FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
					ContextHandle.AddSourceObject(this);
					ContextHandle.AddInstigator(SourcePawn, SourcePawn);

					FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
						DotDamageEffect,
						1.0f,
						ContextHandle
						);

					if (SpecHandle.IsValid())
					{
						SpecHandle.Data->SetSetByCallerMagnitude(
							CMGameplayTags::Shared_SetByCaller_BaseDamage,
							FinalDamage
							);

						SpecHandle.Data->SetSetByCallerMagnitude(
							CMGameplayTags::Shared_SetByCaller_GroggyDamage,
							BaseGroggyDamage
							);

						SpecHandle.Data->SetSetByCallerMagnitude(
							CMGameplayTags::Player_SetByCaller_AttackType_Light,
							0.0f
							);

						TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}
		}
	}
}