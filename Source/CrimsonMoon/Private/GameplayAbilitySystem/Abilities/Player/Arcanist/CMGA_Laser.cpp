// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Player/Arcanist/CMGA_Laser.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "GameplayAbilitySystem/Task/AbilityTask_WaitRepeat.h"
#include "GameplayTags/CMGameplayTags_Player.h"
#include "Kismet/KismetSystemLibrary.h"

UCMGA_Laser::UCMGA_Laser()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UCMGA_Laser::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FVector InvalidLocation(FLT_MAX, FLT_MAX, FLT_MAX);
	LastSentTargetLocation = InvalidLocation;
	LastSyncedLocation = InvalidLocation;

	// 몽타주 재생
	if (LaserMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			LaserMontage,
			1.0f
			);

		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->ReadyForActivation();
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

	// 레이저 시작 이벤트 대기
	if (LaserStartEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* WaitLaserStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			LaserStartEventTag
			);
		WaitLaserStartTask->EventReceived.AddDynamic(this, &ThisClass::OnLaserStartEvent);
		WaitLaserStartTask->ReadyForActivation();
	}
	else
	{
		// 이벤트 태그가 없으면 바로 레이저 로직 시작 (하위 호환성)
		FGameplayEventData DummyPayload;
		OnLaserStartEvent(DummyPayload);
	}
}

void UCMGA_Laser::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (GameplayCueTag.IsValid())
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(GameplayCueTag);
	}

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

	if (LoopSoundEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// 이펙트 제거
			ASC->RemoveActiveGameplayEffect(LoopSoundEffectHandle);
		}

		// 핸들 초기화
		LoopSoundEffectHandle.Invalidate();
	}

	if (LaserMontage && GetCurrentMontage() == LaserMontage)
	{
		MontageJumpToSection(FName("End"));
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMGA_Laser::OnTick()
{
	if (IsLocallyControlled())
	{
		const FVector CurrentAimLocation = GetCameraAimLocation();

		if (FVector::DistSquared(CurrentAimLocation, LastSentTargetLocation) > MIN_ReplicationDistSquared)
		{
			Server_ProcessLaserTick(CurrentAimLocation);
			LastSentTargetLocation = CurrentAimLocation;
		}
	}

	if (GetOwningActorFromActorInfo()->HasAuthority())
	{
		// 아직 클라이언트로부터 유효한 위치를 받지 못했다면 데미지 처리를 하지 않음
		if (LastSyncedLocation.X < FLT_MAX)
		{
			ApplyDamageToServer(LastSyncedLocation);
		}
	}
}

void UCMGA_Laser::Server_ProcessLaserTick_Implementation(const FVector& TargetHitLocation)
{
	// 위치가 유의미하게 변했을 때만 전파
	if (FVector::DistSquared(TargetHitLocation, LastSyncedLocation) > MIN_ReplicationDistSquared)
	{
		LastSyncedLocation = TargetHitLocation;

		FGameplayCueParameters CueParams;
		CueParams.Location = TargetHitLocation;
		CueParams.Instigator = GetAvatarActorFromActorInfo();

		GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(GameplayCueTag, CueParams);
	}
}

FVector UCMGA_Laser::GetCameraAimLocation() const
{
	ACMPlayerCharacterBase* MyCharacter = GetCMPlayerCharacterFromActorInfo();

	// 먼저 TargetLock이 활성화되어 있고 잠긴 타겟이 있는지 확인
	UPlayerCombatComponent* CombatComp = MyCharacter->GetPawnCombatComponent();
	AActor* LockedTarget = CombatComp ? CombatComp->CurrentLockOnTarget.Get() : nullptr;

	if (LockedTarget)
	{
		// 잠긴 타겟이 있으면 타겟의 위치 반환
		return LockedTarget->GetActorLocation();
	}

	// 잠긴 타겟이 없으면 기존 카메라 라인 트레이스 사용
	APlayerController* PC = MyCharacter->GetController<APlayerController>();
	if (!PC)
	{
		return GetAvatarActorFromActorInfo()->GetActorForwardVector() * MaxLaserRange;
	}

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector TraceEnd = CamLoc + (CamRot.Vector() * MaxLaserRange);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetAvatarActorFromActorInfo());

	GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, TraceChannel, Params);

	return Hit.bBlockingHit ? Hit.Location : TraceEnd;
}

void UCMGA_Laser::ApplyDamageToServer(const FVector& TargetLocation)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}

	FVector StartLocation = AvatarActor->GetActorLocation();
	if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (Mesh->DoesSocketExist(MuzzleSocketName))
			{
				StartLocation = Mesh->GetSocketLocation(MuzzleSocketName);
			}
		}
	}

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);

	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		StartLocation,
		TargetLocation,
		LaserTraceRadius,
		TraceTypeQuery2,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
		);

	// 적중 시 데미지 적용
	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();

		if (!OwnerCharacter || !DamageEffect)
		{
			return;
		}

		UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();
		UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();

		if (!PlayerCombatComponent || !ASC)
		{
			return;
		}

		const float BaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel()) * DamageMultiplier;
		const float BaseGroggyDamage = PlayerCombatComponent->GetPlayerCurrentWeaponHeavyGroggyDamage(GetAbilityLevel());

		FGameplayEffectSpecHandle SpecHandle = MakeDamageEffectSpecHandle(
			DamageEffect,
			BaseDamage,
			BaseGroggyDamage,
			CMGameplayTags::Player_SetByCaller_AttackType_Light,
			0
			);

		if (SpecHandle.IsValid())
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get(), ASC->ScopedPredictionKey);
			}
		}
	}
}

void UCMGA_Laser::OnMontageCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("UCMGA_Laser::OnMontageCompleted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMGA_Laser::OnMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("UCMGA_Laser::OnMontageCancelled"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMGA_Laser::OnLaserStartEvent(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("UCMGA_Laser::OnLaserStartEvent"));

	// 레이저 시작 - GameplayCue 추가 및 WaitRepeat 태스크 시작
	FVector InitialLocation;

	if (IsLocallyControlled())
	{
		InitialLocation = GetCameraAimLocation();
	}
	else
	{
		AActor* Avatar = GetAvatarActorFromActorInfo();
		if (Avatar)
		{
			InitialLocation = Avatar->GetActorLocation() + (Avatar->GetActorForwardVector() * MaxLaserRange);
		}
	}

	FGameplayCueParameters CueParams;
	CueParams.Location = InitialLocation;
	CueParams.Instigator = GetAvatarActorFromActorInfo();
	// 레이저 지속 시간을 전달
	CueParams.RawMagnitude = LaserDuration;

	K2_AddGameplayCueWithParams(GameplayCueTag, CueParams, false);

	if (LoopSoundEffectClass)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// GE Spec 생성
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(LoopSoundEffectClass, GetAbilityLevel(), ContextHandle);

			if (SpecHandle.IsValid())
			{
				// GE 적용 및 핸들 저장 (EndAbility 시 초기화)
				LoopSoundEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	if (IsLocallyControlled())
	{
		Server_ProcessLaserTick(InitialLocation);
		LastSentTargetLocation = InitialLocation;
	}

	UAbilityTask_WaitRepeat* Task = UAbilityTask_WaitRepeat::WaitRepeat(this, DamageTickInterval, LaserDuration, true);
	Task->OnTick.AddDynamic(this, &ThisClass::OnTick);
	Task->OnFinished.AddDynamic(this, &ThisClass::K2_EndAbility);
	Task->ReadyForActivation();
}