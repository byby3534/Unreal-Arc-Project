// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_Roll.h"

#include "CMGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "MotionWarpingComponent.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Components/Combat/PlayerCombatComponent.h"

UCMPlayerAbility_Roll::UCMPlayerAbility_Roll()
{
	// 어빌리티 태그 설정
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Rolling);
	SetAssetTags(TagsToAdd);

	ActivationOwnedTags.AddTag(CMGameplayTags::Player_Status_Rolling);

	ActivationBlockedTags.AddTag(CMGameplayTags::Player_Status_Rolling);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UCMPlayerAbility_Roll::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 스태미나 비용 체크 및 소모
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACMPlayerCharacterBase* PlayerCharacterBase = CastChecked<ACMPlayerCharacterBase>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacterBase)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 캐싱된 입력 방향 사용 (Roll 버튼 누를 때 저장된 값, 네트워크를 통해 복제됨)
	const FVector WorldMovementInput = PlayerCharacterBase->GetCachedRollInputDirection();

	// 컨트롤 로테이션 기준으로 월드 스페이스 입력을 로컬 스페이스로 변환
	const FRotator ControlRotation = PlayerCharacterBase->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector LocalMovementInput = YawRotation.UnrotateVector(WorldMovementInput);

	const FVector2D InputVector = FVector2D(LocalMovementInput.X, LocalMovementInput.Y);

	// 입력이 없으면 전방으로 롤링
	const FVector2D RollDirectionVector = InputVector.IsNearlyZero() ? FVector2D(1.0f, 0.0f) : InputVector.GetSafeNormal();

	// 롤링 방향 계산
	const ERollDirection RollDirection = CalculateRollDirection(RollDirectionVector);

	const FVector ForwardDir = YawRotation.RotateVector(FVector::ForwardVector);
	const FVector RightDir = YawRotation.RotateVector(FVector::RightVector);
	const FVector WorldRollDirection = (ForwardDir * RollDirectionVector.X + RightDir * RollDirectionVector.Y).GetSafeNormal();

	// 롤링 거리 계산
	const FVector StartLocation = PlayerCharacterBase->GetActorLocation();
	const float SafeRollDistance = CalculateSafeRollDistance(StartLocation, WorldRollDirection);

	// 목표 위치 계산
	const FVector TargetLocation = StartLocation + (WorldRollDirection * SafeRollDistance);

	// 무기 장착 여부 확인
	const UPlayerCombatComponent* PlayerCombatComponent = PlayerCharacterBase->GetPawnCombatComponent();
	const bool bIsWeaponEquipped = PlayerCombatComponent && PlayerCombatComponent->GetPlayerCurrentEquippedWeapon() != nullptr;

	const TMap<ERollDirection, TObjectPtr<UAnimMontage>>& SelectedMontages = bIsWeaponEquipped ? DodgeMontages : RollMontages;

	const TObjectPtr<UAnimMontage>* FoundMontage = SelectedMontages.Find(RollDirection);

	if (!FoundMontage || !(*FoundMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = *FoundMontage;

	// 모션 워핑 설정
	SetupMotionWarping(TargetLocation);

	// 무적 이펙트 적용
	if (InvincibilityEffect)
	{
		if (UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo())
		{
			InvincibilityEffectHandle = ASC->ApplyGameplayEffectToSelf(
				InvincibilityEffect->GetDefaultObject<UGameplayEffect>(),
				1.0f,
				ASC->MakeEffectContext()
				);
		}
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		NAME_None,
		false
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UCMPlayerAbility_Roll::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 무적 이펙트 제거
	if (InvincibilityEffectHandle.IsValid())
	{
		if (UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(InvincibilityEffectHandle);
		}
		InvincibilityEffectHandle.Invalidate();
	}

	// 몽타주 태스크 정리
	if (MontageTask && MontageTask->IsActive())
	{
		MontageTask->EndTask();
	}
	MontageTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UCMPlayerAbility_Roll::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (RollMontages.IsEmpty() && DodgeMontages.IsEmpty())
	{
		return false;
	}

	return true;
}

void UCMPlayerAbility_Roll::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_Roll::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

ERollDirection UCMPlayerAbility_Roll::CalculateRollDirection(const FVector2D& InputVector)
{
	float Angle = FMath::Atan2(InputVector.Y, InputVector.X) * (180.0f / PI);
	if (Angle < 0.0f)
	{
		Angle += 360.0f;
	}

	// 8방향으로 스냅 (45도씩 8개 영역)
	const int32 DirectionIndex = FMath::RoundToInt(Angle / 45.0f) % 8;

	switch (DirectionIndex)
	{
		case 0:
			return ERollDirection::Forward;

		case 1:
			return ERollDirection::ForwardRight;

		case 2:
			return ERollDirection::Right;

		case 3:
			return ERollDirection::BackwardRight;

		case 4:
			return ERollDirection::Backward;

		case 5:
			return ERollDirection::BackwardLeft;

		case 6:
			return ERollDirection::Left;

		case 7:
			return ERollDirection::ForwardLeft;

		default:
			return ERollDirection::Forward;
	}
}

float UCMPlayerAbility_Roll::CalculateSafeRollDistance(const FVector& StartLocation, const FVector& Direction) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return DefaultRollDistance;
	}

	const ACMPlayerCharacterBase* PlayerCharacterBase = Cast<ACMPlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacterBase)
	{
		return DefaultRollDistance;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerCharacterBase);
	QueryParams.bTraceComplex = false;

	FCollisionObjectQueryParams ObjectQueryParams;
	for (const auto& ObjectType : TraceObjectTypes)
	{
		ObjectQueryParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
	}

	// ObjectType이 비어있으면 WorldStatic 추가
	if (TraceObjectTypes.Num() == 0)
	{
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	}

	const float CapsuleHalfHeight = PlayerCharacterBase->GetCapsuleComponent() ? PlayerCharacterBase->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 90.0f;

	const FVector HorizontalTraceStart = StartLocation + FVector(0.0f, 0.0f, CapsuleHalfHeight * 0.5f);
	const FVector HorizontalTraceEnd = HorizontalTraceStart + (Direction * DefaultRollDistance);

	FHitResult HorizontalHit;
	const bool bHitObstacle = World->LineTraceSingleByObjectType(
		HorizontalHit,
		HorizontalTraceStart,
		HorizontalTraceEnd,
		ObjectQueryParams,
		QueryParams
		);

	// 장애물이 있으면 그 전까지만 이동
	float SafeHorizontalDistance = DefaultRollDistance;
	if (bHitObstacle)
	{
		const float ObstacleDistance = (HorizontalHit.Location - HorizontalTraceStart).Size();
		SafeHorizontalDistance = FMath::Max(ObstacleDistance - 50.0f, 100.0f); // 장애물 50cm 전, 최소 100cm

		if (bDebugDrawTrace)
		{
			DrawDebugLine(World, HorizontalTraceStart, HorizontalHit.Location, FColor::Green, false, 3.0f, 0, 3.0f);
			DrawDebugLine(World, HorizontalHit.Location, HorizontalTraceEnd, FColor::Red, false, 3.0f, 0, 3.0f);
			DrawDebugSphere(World, HorizontalHit.Location, 20.0f, 12, FColor::Red, false, 3.0f, 0, 3.0f);
		}
	}
	else if (bDebugDrawTrace)
	{
		DrawDebugLine(World, HorizontalTraceStart, HorizontalTraceEnd, FColor::Green, false, 3.0f, 0, 3.0f);
	}

	// 2: 수직 트레이스 - 목표 위치의 바닥/절벽 확인
	const FVector TargetHorizontalLocation = StartLocation + (Direction * SafeHorizontalDistance);
	const FVector VerticalTraceStart = TargetHorizontalLocation + FVector(0.0f, 0.0f, 200.0f);
	const FVector VerticalTraceEnd = TargetHorizontalLocation - FVector(0.0f, 0.0f, 500.0f);

	FHitResult VerticalHit;
	const bool bHitGround = World->LineTraceSingleByObjectType(
		VerticalHit,
		VerticalTraceStart,
		VerticalTraceEnd,
		ObjectQueryParams,
		QueryParams
		);

	// 디버그: 수직 트레이스
	if (bDebugDrawTrace)
	{
		DrawDebugSphere(World, HorizontalTraceStart, 15.0f, 12, FColor::Blue, false, 3.0f, 0, 3.0f);
		DrawDebugSphere(World, TargetHorizontalLocation, 15.0f, 12, FColor::Yellow, false, 3.0f, 0, 3.0f);

		if (bHitGround)
		{
			DrawDebugLine(World, VerticalTraceStart, VerticalHit.Location, FColor::Cyan, false, 3.0f, 0, 3.0f);
			DrawDebugLine(World, VerticalHit.Location, VerticalTraceEnd, FColor::Magenta, false, 3.0f, 0, 3.0f);
			DrawDebugSphere(World, VerticalHit.Location, 20.0f, 12, FColor::Orange, false, 3.0f, 0, 3.0f);
		}
		else
		{
			DrawDebugLine(World, VerticalTraceStart, VerticalTraceEnd, FColor::Red, false, 3.0f, 0, 3.0f);
		}
	}

	// 바닥이 없으면 (절벽) 거리를 절반으로 제한
	if (!bHitGround)
	{
		const float CliffSafeDistance = SafeHorizontalDistance * 0.5f;

		return CliffSafeDistance;
	}

	// 높이 차이 계산 - 발바닥 기준으로 정확한 낙차 계산
	const float FeetLocationZ = StartLocation.Z - CapsuleHalfHeight;
	const float HeightDifference = FeetLocationZ - VerticalHit.Location.Z;

	if (HeightDifference > 150.0f)
	{
		return 10.0f;
	}

	return SafeHorizontalDistance;
}

void UCMPlayerAbility_Roll::SetupMotionWarping(const FVector& TargetLocation) const
{
	const ACMPlayerCharacterBase* PlayerCharacterBase = Cast<ACMPlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacterBase)
	{
		return;
	}

	UMotionWarpingComponent* MotionWarping = PlayerCharacterBase->FindComponentByClass<UMotionWarpingComponent>();
	if (!MotionWarping)
	{
		return;
	}

	// 목표 위치를 향한 회전 계산 (Pitch, Roll 제거)
	FRotator TargetRotation = (TargetLocation - PlayerCharacterBase->GetActorLocation()).Rotation();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;

	// 모션 워핑 타겟 추가/업데이트
	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
		WarpTargetName,
		TargetLocation,
		TargetRotation
		);
}