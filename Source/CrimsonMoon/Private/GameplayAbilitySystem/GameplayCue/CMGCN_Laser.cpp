// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/GameplayCue/CMGCN_Laser.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

ACMGCN_Laser::ACMGCN_Laser()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bAutoDestroyOnRemove = true;

	LaserNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LaserNiagaraComponent"));
	LaserNiagaraComponent->bAutoActivate = false;
	RootComponent = LaserNiagaraComponent;

	LaserNiagaraComponent->OnSystemFinished.AddDynamic(
		this,
		&ACMGCN_Laser::OnLaserSystemFinished
		);
}

void ACMGCN_Laser::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateLaserTransform(DeltaSeconds);
}

void ACMGCN_Laser::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);

	switch (EventType)
	{
		case EGameplayCueEvent::OnActive:
		{
			bAllowAutoRestart = true;

			TargetBeamEndLocation = Parameters.Location;
			CurrentSmoothEndLocation = Parameters.Location;

			if (Parameters.RawMagnitude > 0.0f)
			{
				CachedLaserDuration = Parameters.RawMagnitude;
			}

			// 내 캐릭터거나 AI,서버인 경우에만 즉시 킴.
			// 남의 캐릭터는 데이터가 올 때까지 대기
			bool bShouldActivateImmediately = false;
			if (APawn* PawnTarget = Cast<APawn>(MyTarget))
			{
				if (PawnTarget->IsLocallyControlled())
				{
					bShouldActivateImmediately = true;
				}
			}

			if (bShouldActivateImmediately)
			{
				UpdateLaserTransform(0.0f);

				if (LaserNiagaraComponent)
				{
					LaserNiagaraComponent->DeactivateImmediate();
					LaserNiagaraComponent->SetFloatParameter(LaserDurationParamName, CachedLaserDuration);

					LaserNiagaraComponent->Activate(true);
				}
			}
			SetActorTickEnabled(true);
			break;
		}
		case EGameplayCueEvent::Executed:
			TargetBeamEndLocation = Parameters.Location;
			if (LaserNiagaraComponent && !LaserNiagaraComponent->IsActive())
			{
				CurrentSmoothEndLocation = TargetBeamEndLocation;
				UpdateLaserTransform(0.0f);

				LaserNiagaraComponent->SetFloatParameter(LaserDurationParamName, CachedLaserDuration);

				LaserNiagaraComponent->Activate(true);
			}
			break;

		case EGameplayCueEvent::Removed:
		{
			bAllowAutoRestart = false;

			if (LaserNiagaraComponent && LaserNiagaraComponent->IsActive())
			{
				LaserNiagaraComponent->DeactivateImmediate();
			}
			CachedLaserDuration = 1.0f;
			SetActorTickEnabled(false);
			break;
		}
	}
}

void ACMGCN_Laser::UpdateLaserTransform(float DeltaTime)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !LaserNiagaraComponent)
	{
		return;
	}

	// 시작점 위치 구하기
	FVector StartLocation = OwnerCharacter->GetActorLocation();
	if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
	{
		if (Mesh->DoesSocketExist(MuzzleSocketName))
		{
			StartLocation = Mesh->GetSocketLocation(MuzzleSocketName);
		}
	}

	// 목표 지점 보간
	// DeltaTime이 0이면 보간 없이 즉시 목표값 적용 (튀는 현상 방지)
	if (DeltaTime > 0.0f)
	{
		CurrentSmoothEndLocation = FMath::VInterpTo(CurrentSmoothEndLocation, TargetBeamEndLocation, DeltaTime, 20.0f);
	}
	else
	{
		CurrentSmoothEndLocation = TargetBeamEndLocation;
	}
	// 액터 이동
	SetActorLocation(StartLocation);

	// 액터 회전
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, CurrentSmoothEndLocation);
	SetActorRotation(LookAtRotation);
	// 길이 파라미터 업데이트
	float LaserLength = FVector::Dist(StartLocation, CurrentSmoothEndLocation);

	LaserNiagaraComponent->SetVectorParameter(LaserLengthParamName, FVector(LaserLength, 0, 0));

	// #if UE_BUILD_SHIPPING == 0
	// DrawDebugSphere(GetWorld(), StartLocation, 10.0f, 12, FColor::Red, false, -1.0f);
	// DrawDebugSphere(GetWorld(), CurrentSmoothEndLocation, 5.0f, 12, FColor::Purple, false, -1.0f);
	// DrawDebugLine(GetWorld(), StartLocation, CurrentSmoothEndLocation, FColor::Green, false, -1.0f, 0, 1.0f);
	// #endif
}

void ACMGCN_Laser::OnLaserSystemFinished(UNiagaraComponent* FinishedComponent)
{
	if (!bAllowAutoRestart || !LaserNiagaraComponent)
	{
		return;
	}

	LaserNiagaraComponent->DeactivateImmediate();
	LaserNiagaraComponent->SetFloatParameter(LaserDurationParamName, CachedLaserDuration);
	LaserNiagaraComponent->Activate(true);
}