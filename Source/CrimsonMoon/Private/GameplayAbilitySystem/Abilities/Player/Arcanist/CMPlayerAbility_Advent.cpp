// Copyright CrimsonMoon Team. All Rights Reserved.

#include "GameplayAbilitySystem/Abilities/Player/Arcanist/CMPlayerAbility_Advent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CMFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CMGameplayTags.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameplayAbilitySystem/Abilities/Skill/CMAdventActor.h"
#include "GameplayAbilitySystem/Task/AbilityTask_FireProjectile.h"

UCMPlayerAbility_Advent::UCMPlayerAbility_Advent()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Skill_Arcanist_F);
	SetAssetTags(TagsToAdd);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UCMPlayerAbility_Advent::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!SkillMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 커밋에 실패하면 어빌리티 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 로컬 컨트롤러에서만 타겟 위치 계산
	if (IsLocallyControlled())
	{
		// 카메라 라인트레이스로 타겟 감지
		FVector HitLocation = GetCameraTraceTarget();

		// Z값을 바닥으로 조정
		FHitResult GroundHit;
		FVector TraceStart = HitLocation;
		FVector TraceEnd = HitLocation - FVector(0.0f, 0.0f, 10000.0f); // 아래로 충분히 긴 거리

		FCollisionQueryParams GroundParams;
		GroundParams.AddIgnoredActor(GetAvatarActorFromActorInfo());

		bool bGroundHit = GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, GroundParams);
		if (bEnableDebugDraw)
		{
			DrawDebugLine(GetWorld(), TraceStart, bGroundHit ? GroundHit.Location : TraceEnd, bGroundHit ? FColor::Green : FColor::Red, false, DebugDrawDuration, 0, 2.0f);
			if (bGroundHit)
			{
				DrawDebugPoint(GetWorld(), GroundHit.ImpactPoint, 10.0f, FColor::Green, false, DebugDrawDuration);
			}
		}

		if (bGroundHit)
		{
			HitLocation.Z = GroundHit.ImpactPoint.Z;
		}

		// 타겟 위치 저장
		CachedTargetLocation = HitLocation;

		// 서버에 타겟 위치 전달
		ServerSetTargetLocation(CachedTargetLocation);
	}

	if (StartSoundEffectClass)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// GE Spec 생성
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StartSoundEffectClass, GetAbilityLevel(), ContextHandle);

			if (SpecHandle.IsValid())
			{
				// GE 적용 및 핸들 저장 (EndAbility 시 초기화)
				StartSoundEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		// 천둥번개 타격 이벤트 대기
		UAbilityTask_WaitGameplayEvent* WaitThunderStrikeTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			CMGameplayTags::Player_Event_Advent_ThunderStrike
			);
		WaitThunderStrikeTask->EventReceived.AddDynamic(this, &ThisClass::OnThunderStrikeEvent);
		WaitThunderStrikeTask->ReadyForActivation();
	}

	// 몽타주 재생
	if (SkillMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			SkillMontage,
			1.0f
			);

		// 몽타주 완료 시 어빌리티 종료 (도트 데미지는 CMAdventActor에서 처리)
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 몽타주가 없으면 바로 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UCMPlayerAbility_Advent::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (StartSoundEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// 이펙트 제거
			ASC->RemoveActiveGameplayEffect(StartSoundEffectHandle);
		}

		// 핸들 초기화
		StartSoundEffectHandle.Invalidate();
	}

	if (ThunderSoundEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// 이펙트 제거
			ASC->RemoveActiveGameplayEffect(ThunderSoundEffectHandle);
		}

		// 핸들 초기화
		ThunderSoundEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UCMPlayerAbility_Advent::GetCameraTraceTarget() const
{
	ACMPlayerCharacterBase* PlayerCharacter = GetCMPlayerCharacterFromActorInfo();

	// 먼저 TargetLock이 활성화되어 있고 잠긴 타겟이 있는지 확인
	UPlayerCombatComponent* CombatComp = PlayerCharacter->GetPawnCombatComponent();
	AActor* LockedTarget = CombatComp ? CombatComp->CurrentLockOnTarget.Get() : nullptr;

	if (LockedTarget)
	{
		// 잠긴 타겟이 있으면 타겟의 위치 반환
		return LockedTarget->GetActorLocation();
	}

	// 잠긴 타겟이 없으면 기존 카메라 라인 트레이스 사용
	APlayerController* PC = PlayerCharacter ? PlayerCharacter->GetController<APlayerController>() : nullptr;

	FHitResult HitResult;

	if (!PC)
	{
		return HitResult.Location;
	}

	// 카메라 위치 방향
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector CameraTraceEnd = CamLoc + (CamRot.Vector() * MaxTraceDistance);

	// 라인트레이스 설정
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetAvatarActorFromActorInfo());

	// 라인트레이스 실행
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CamLoc,
		CameraTraceEnd,
		TraceChannel,
		Params
		);

	if (bEnableDebugDraw)
	{
		DrawDebugLine(GetWorld(), CamLoc, bHit ? HitResult.Location : CameraTraceEnd, bHit ? FColor::Green : FColor::Red, false, DebugDrawDuration, 0, 2.0f);
		if (bHit)
		{
			DrawDebugPoint(GetWorld(), HitResult.Location, 10.0f, FColor::Green, false, DebugDrawDuration);
		}
	}

	return bHit ? HitResult.Location : CameraTraceEnd;
}

void UCMPlayerAbility_Advent::ServerSetTargetLocation_Implementation(FVector_NetQuantize TargetLocation)
{
	// 서버에서 클라이언트가 계산한 타겟 위치 저장
	CachedTargetLocation = TargetLocation;
}

void UCMPlayerAbility_Advent::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_Advent::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMPlayerAbility_Advent::OnThunderStrikeEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	// 1. 초기 폭발 데미지 적용
	ApplyInitialExplosionDamage(CachedTargetLocation);

	// 2. GameplayCue 실행 (천둥번개 VFX 및 사운드)
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (SourceASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = CachedTargetLocation;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.RawMagnitude = DotDuration;

		// VFX (일회성)
		SourceASC->ExecuteGameplayCue(
			CMGameplayTags::GameplayCue_Arcanist_Advent_VFX,
			CueParams
			);

		if (ThunderSoundEffectClass)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
			if (ASC)
			{
				// GE Spec 생성
				FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
				ContextHandle.AddSourceObject(this);

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ThunderSoundEffectClass, GetAbilityLevel(), ContextHandle);

				if (SpecHandle.IsValid())
				{
					// GE 적용 및 핸들 저장 (EndAbility 시 초기화)
					ThunderSoundEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}

	// 3. FireProjectile 태스크로 장판 액터 소환
	if (AdventActorClass)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(CachedTargetLocation);

		// 초기 폭발 데미지용으로 BaseDamage 계산
		ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();
		if (OwnerCharacter)
		{
			UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();
			const float BaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel());

			UAbilityTask_FireProjectile* FireTask = UAbilityTask_FireProjectile::FireProjectile(
				this,
				AdventActorClass,
				SpawnTransform,
				DotDamageEffect,
				BaseDamage
				);

			FireTask->OnSuccess.AddDynamic(this, &ThisClass::OnAdventActorSpawned);
			FireTask->ReadyForActivation();
		}
	}
}

void UCMPlayerAbility_Advent::OnAdventActorSpawned(ACMProjectileActor* SpawnedActor)
{
	// 장판 액터 추가 설정
	if (ACMAdventActor* AdventActor = Cast<ACMAdventActor>(SpawnedActor))
	{
		AdventActor->SetDotDamageInfo(
			DotDamageEffect,
			DotDamageMultiplier,
			DotInterval,
			DotDuration,
			ExplosionRadius
			);
		AdventActor->StartDotTimer();
		AdventActor->SetDebugDrawSettings(bEnableDebugDraw, DebugDrawDuration);
	}
}

void UCMPlayerAbility_Advent::ApplyInitialExplosionDamage(const FVector& Location)
{
	ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();
	if (!OwnerCharacter)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	// SphereOverlap으로 범위 내 적 탐색
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(OwnerCharacter);
	TArray<AActor*> OverlappedActors;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Location,
		ExplosionRadius,
		ObjectTypes,
		nullptr,
		IgnoredActors,
		OverlappedActors
		);

	// 초기 폭발 범위 디버그 드로우 (빨간색)
	if (bEnableDebugDraw)
	{
		DrawDebugSphere(
			GetWorld(),
			Location,
			ExplosionRadius,
			32,
			FColor::Red,
			false,
			DebugDrawDuration,
			0,
			2.0f
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
			APawn* SourcePawn = Cast<APawn>(OwnerCharacter);
			APawn* TargetPawn = Cast<APawn>(HitActor);
			if (UCMFunctionLibrary::IsTargetPawnHostile(SourcePawn, TargetPawn))
			{
				// 데미지 적용
				if (InitialDamageEffect)
				{
					UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();
					const float BaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel()) * InitialDamageMultiplier;
					const float BaseGroggyDamage = PlayerCombatComponent->GetPlayerCurrentWeaponHeavyGroggyDamage(GetAbilityLevel());

					FGameplayEffectSpecHandle DamageSpecHandle = MakeDamageEffectSpecHandle(
						InitialDamageEffect,
						BaseDamage,
						BaseGroggyDamage,
						CMGameplayTags::Player_SetByCaller_AttackType_Light,
						0
						);

					if (DamageSpecHandle.IsValid())
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
					}
				}
			}
		}
	}
}