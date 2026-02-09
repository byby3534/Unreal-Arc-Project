// Copyright CrimsonMoon Team. All Rights Reserved.

#include "GameplayAbilitySystem/Abilities/Player/Blade/CMPlayerAbility_Issen_Blade.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CMGameplayTags.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "GameplayAbilitySystem/Abilities/Skill/Projectile/CMProjectileActor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "System/CMObjectPoolingManager.h"

UCMPlayerAbility_Issen_Blade::UCMPlayerAbility_Issen_Blade()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Skill_Blade_Q);
	SetAssetTags(TagsToAdd);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UCMPlayerAbility_Issen_Blade::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ChargeMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 커밋에 실패하면 (예: 스태미나 부족) 어빌리티를 즉시 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 차지 시작
	StartCharging();
}

void UCMPlayerAbility_Issen_Blade::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	const bool bHasAuthority = HasAuthority(&ActivationInfo);

	// 어빌리티가 '차징 중에' 취소되거나 종료될 경우를 대비해 태그 정리
	if (bIsCharging)
	{
		if (UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(CMGameplayTags::Player_Status_Charging);
		}
	}

	// 상태 초기화
	bIsCharging = false;
	bIsDashing = false;
	ChargeStartTime = 0.0f;

	// [서버 전용] 차지 레벨 초기화
	if (bHasAuthority)
	{
		CurrentChargeLevel = 0.0f;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMPlayerAbility_Issen_Blade::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!bIsCharging)
	{
		return;
	}

	// 차지 레벨 계산 (클라이언트에서 계산 후 서버로 전송)
	const float ChargeTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	const float CalculatedChargeLevel = FMath::Clamp((ChargeTime - MinChargeTime) / (MaxChargeTime - MinChargeTime), 0.0f, 1.0f);

	// 차지가 최소 시간보다 짧으면 취소
	if (ChargeTime < MinChargeTime)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 몽타주 루프 방지
	bIsCharging = false;

	// [클라이언트 → 서버] 차지 릴리즈 요청
	Server_ExecuteChargeSlash(CalculatedChargeLevel);
}

void UCMPlayerAbility_Issen_Blade::Server_ExecuteChargeSlash_Implementation(float InChargeLevel)
{
	// [서버] 차지 레벨 설정
	CurrentChargeLevel = FMath::Clamp(InChargeLevel, 0.0f, 1.0f);

	bIsCharging = false;

	// [중요] 리슨 서버 호스트(나 자신)에게는 RPC를 보내지 않음
	// IsLocallyControlled()가 true라면, 이 코드를 실행하는 서버가 곧 플레이어(호스트)라는 뜻
	if (!IsLocallyControlled())
	{
		Client_ExecuteChargeSlash(CurrentChargeLevel); // 원격 클라이언트에게만 RPC 전송
	}
	ExecuteChargeSlash(CurrentChargeLevel); // 서버에서 돌진 베기 실행
}

void UCMPlayerAbility_Issen_Blade::Client_ExecuteChargeSlash_Implementation(float InChargeLevel)
{
	// [클라이언트] 서버로부터 차지 레벨 수신 및 일섬 실행
	CurrentChargeLevel = InChargeLevel;

	// 클라이언트에서도 돌진 베기 실행 (몽타주 재생)
	ExecuteChargeSlash(CurrentChargeLevel);
}

void UCMPlayerAbility_Issen_Blade::StartCharging()
{
	bIsCharging = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();
	CurrentChargeLevel = 0.0f;

	// 차징 시작 시 태그 부여
	if (UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->AddLooseGameplayTag(CMGameplayTags::Player_Status_Charging);
	}

	// [서버 & 클라이언트] 차지 몽타주 재생 (AbilityTask 사용 - 자동 복제)
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeMontage);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnChargeMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnChargeMontageCompleted);

	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnChargeMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnChargeMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UCMPlayerAbility_Issen_Blade::ExecuteChargeSlash(float InChargeLevel)
{
	if (!ChargeSlashMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bIsCharging = false;

	if (UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CMGameplayTags::Player_Status_Charging);
	}

	CurrentChargeLevel = InChargeLevel;

	const bool bHasAuthority = HasAuthority(&CurrentActivationInfo);

	// 차지 레벨에 따라 돌진 또는 검기 발사 선택
	if (CurrentChargeLevel >= ChargeThreshold)
	{
		// 고차지: 검기 발사
		// [서버 전용] 검기 발사
		if (bHasAuthority)
		{
			PerformProjectileSlash(CurrentChargeLevel);
		}

		// 검기 발사 시에도 몽타주 재생
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeSlashMontage);

		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnChargeSlashMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnChargeSlashMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 저차지: 돌진 베기 (기존 로직)
		// [서버 전용] 데미지 이벤트 대기
		if (bHasAuthority)
		{
			UAbilityTask_WaitGameplayEvent* WaitHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CMGameplayTags::Shared_Event_MeleeHit);
			WaitHitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitTarget);
			WaitHitTask->ReadyForActivation();
		}

		// [서버 & 클라이언트] 돌진 베기 몽타주 재생 (AbilityTask 사용 - 자동 복제)
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeSlashMontage);

		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnChargeSlashMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnChargeSlashMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		MontageTask->ReadyForActivation();

		// [서버 & 클라이언트] 돌진 시작 (클라이언트 예측)
		PerformDash();
	}
}

void UCMPlayerAbility_Issen_Blade::PerformDash()
{
	bIsDashing = true;

	ACMPlayerCharacterBase* PlayerCharacterBase = CastChecked<ACMPlayerCharacterBase>(GetAvatarActorFromActorInfo());

	// 돌진 방향 계산
	const FVector ForwardVector = PlayerCharacterBase->GetActorForwardVector();

	// [서버 & 클라이언트] 캐릭터 Launch (클라이언트 예측 포함)
	// 서버: 권위 있는 이동 처리
	// 클라이언트: 로컬 예측으로 부드러운 이동
	const FVector LaunchVelocity = ForwardVector * (DashDistance / DashDuration);
	PlayerCharacterBase->LaunchCharacter(LaunchVelocity, true, true);

	// 돌진 종료 타이머
	FTimerHandle DashEndTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DashEndTimerHandle, this, &UCMPlayerAbility_Issen_Blade::OnDashComplete, DashDuration, false);
}

void UCMPlayerAbility_Issen_Blade::OnDashComplete()
{
	// [서버 전용]
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	bIsDashing = false;
}

void UCMPlayerAbility_Issen_Blade::PerformProjectileSlash(float InChargeLevel) const
{
	// [서버 전용]
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	if (!ProjectileClass || !DamageEffect)
	{
		return;
	}

	ACMPlayerCharacterBase* PlayerCharacter = Cast<ACMPlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter)
	{
		return;
	}

	const UPlayerCombatComponent* PlayerCombatComponent = PlayerCharacter->GetPawnCombatComponent();
	if (!PlayerCombatComponent)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 오브젝트 풀링 매니저 가져오기
	UCMObjectPoolingManager* PoolingManager = World->GetSubsystem<UCMObjectPoolingManager>();
	if (!PoolingManager)
	{
		return;
	}

	// 차지 레벨에 따라 속도 계산 (ChargeThreshold ~ 1.0 -> MinProjectileSpeed ~ MaxProjectileSpeed)
	const float NormalizedChargeLevel = (InChargeLevel - ChargeThreshold) / (1.0f - ChargeThreshold);
	const float ProjectileSpeed = FMath::Lerp(MinProjectileSpeed, MaxProjectileSpeed, NormalizedChargeLevel);

	// Projectile 스폰 위치 및 방향 설정
	const FVector SpawnLocation = PlayerCharacter->GetActorLocation() + PlayerCharacter->GetActorForwardVector() * 100.0f;
	const FRotator SpawnRotation = PlayerCharacter->GetActorRotation();
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	// 오브젝트 풀에서 Projectile 가져오기
	ACMProjectileActor* Projectile = Cast<ACMProjectileActor>(PoolingManager->GetObject(ProjectileClass));
	if (!Projectile)
	{
		return;
	}

	// Owner와 Instigator 설정
	Projectile->SetOwner(PlayerCharacter);
	Projectile->SetInstigator(PlayerCharacter);

	// Transform 설정
	Projectile->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::ResetPhysics);

	// 속도 설정
	if (UProjectileMovementComponent* ProjectileMovement = Projectile->GetProjectileMovement())
	{
		ProjectileMovement->InitialSpeed = ProjectileSpeed;
		ProjectileMovement->MaxSpeed = ProjectileSpeed;
	}

	// 데미지 정보 설정
	const float WeaponBaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel());
	Projectile->SetDamageInfo(DamageEffect, WeaponBaseDamage);

	// Hit GameplayCue 태그 설정
	if (GameplayCueTag.IsValid())
	{
		Projectile->SetHitGameplayCueTag(GameplayCueTag);
	}

	// Projectile 이동 초기화
	Projectile->InitProjectileTransform(SpawnTransform);

	// 오브젝트 활성화
	PoolingManager->ActivateObject(Projectile);

	// LifeSpan 설정 (거리 / 속도)
	const float LifeSpan = ProjectileMaxDistance / ProjectileSpeed;
	Projectile->SetLifeSpan(LifeSpan);
}

void UCMPlayerAbility_Issen_Blade::OnChargeMontageCompleted()
{
	// 여전히 차지 중이라면 (사용자가 아직 버튼을 누르고 있음)
	if (bIsCharging)
	{
		// 최대 차지 시간에 도달했는지 확인
		const float ChargeTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
		if (ChargeTime >= MaxChargeTime)
		{
			// 최대 차지 시간 도달 - 자동 릴리즈
			constexpr float CalculatedChargeLevel = 1.0f; // 100% 차지
			bIsCharging = false;                          // 차지 종료 상태로 변경

			// 서버와 클라이언트에서 다르게 처리
			if (HasAuthority(&CurrentActivationInfo))
			{
				// 서버에서는 Implementation 직접 호출
				Server_ExecuteChargeSlash_Implementation(CalculatedChargeLevel);
			}
			else
			{
				// 클라이언트에서는 RPC 호출
				Server_ExecuteChargeSlash(CalculatedChargeLevel);
			}
		}
		else
		{
			// 차지 몽타주를 다시 재생 (루프)
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, ChargeMontage);

			MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnChargeMontageCompleted);
			MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnChargeMontageCompleted);

			MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnChargeMontageCancelled);
			MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnChargeMontageCancelled);
			MontageTask->ReadyForActivation();
		}
	}
}

void UCMPlayerAbility_Issen_Blade::OnChargeSlashMontageCompleted()
{
	// 돌진 베기 몽타주가 정상 종료됨
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_Issen_Blade::OnChargeMontageCancelled()
{
	// bIsCharging이 true라면: 적에게 맞거나 해서 진짜로 취소된 상황 -> 종료
	// bIsCharging이 false라면: ExecuteChargeSlash로 넘어가면서 상태가 바뀐 상황 -> 종료하지 않음 (무시)
	if (!bIsCharging)
	{
		return;
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMPlayerAbility_Issen_Blade::OnMontageCancelled()
{
	// 몽타주가 취소됨 (피격, 회피 등)
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMPlayerAbility_Issen_Blade::OnHitTarget(FGameplayEventData Payload)
{
	// [서버 전용] AnimNotify에서 발생한 MeleeHit 이벤트 처리
	// 이 함수는 서버의 WaitHitTask에 의해서만 호출되므로 HasAuthority 검사 불필요

	const AActor* HitActor = Payload.Target.Get();
	if (!HitActor || !DamageEffect)
	{
		return;
	}

	// 캐릭터 및 컴포넌트 가져오기
	ACMPlayerCharacterBase* OwnerCharacter = Cast<ACMPlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter)
	{
		return;
	}

	UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();
	if (!PlayerCombatComponent)
	{
		return;
	}

	// 무기 기본 데미지 가져오기
	const float WeaponBaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel());
	const float BaseGroggyDamage = PlayerCombatComponent->GetPlayerCurrentWeaponHeavyGroggyDamage(GetAbilityLevel());

	// DamageEffect 스펙 생성 (헤비 공격으로 처리, 콤보 카운트는 차지 레벨 기반)
	// 차지 레벨을 콤보 카운트로 변환 (0~1 → 1~4)
	const int32 ComboCountFromCharge = FMath::CeilToInt(CurrentChargeLevel * 4.0f);
	const float GroggyDamage = BaseGroggyDamage * FMath::Lerp(1.0f, MaxChargeGroggyMultiplier, CurrentChargeLevel);

	FGameplayEffectSpecHandle SpecHandle = MakeDamageEffectSpecHandle(
		DamageEffect,
		WeaponBaseDamage,
		GroggyDamage,
		CMGameplayTags::Player_SetByCaller_AttackType_Heavy, // 헤비 공격 태그 사용
		ComboCountFromCharge
		);

	ExecuteWeaponHitGameplayCue(HitActor, GameplayCueTag);

	if (SpecHandle.IsValid())
	{
		// 타겟의 ASC 가져오기
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(HitActor));
		if (TargetASC)
		{
			// 타겟에 데미지 적용
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get(), GetAbilitySystemComponentFromActorInfo()->ScopedPredictionKey);

			// 타겟의 ASC에서 Hit Sound Gameplay Cue 실행
			ExecuteTargetHitGameplayCue(HitActor, CMGameplayTags::GameplayCue_Sounds_MeleeHit_Blade);
		}
	}
}