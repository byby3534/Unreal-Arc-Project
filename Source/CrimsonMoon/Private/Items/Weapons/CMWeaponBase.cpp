// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/CMWeaponBase.h"
#include "CMFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"

ACMWeaponBase::ACMWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // 이 액터를 복제하도록 설정

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetIsReplicated(true);
	SetRootComponent(WeaponMesh);

	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Collision Box"));
	WeaponCollisionBox->SetupAttachment(GetRootComponent());
	WeaponCollisionBox->SetBoxExtent(FVector(20.f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnCollisionBoxBeginOverlap);
	WeaponCollisionBox->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnCollisionBoxEndOverlap);
}

void ACMWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// bHideUntilEquipped가 true인 경우 초기 상태를 숨김으로 설정
	if (bHideUntilEquipped)
	{
		HideWeapon();
	}
}

void ACMWeaponBase::AddGrantedGameplayEffect(FActiveGameplayEffectHandle Handle)
{
	GrantedEffectHandles.Add(Handle);
}

TArray<FActiveGameplayEffectHandle> ACMWeaponBase::RemoveGrantedGameplayEffects()
{
	// 복사본을 만들어서 반환
	TArray<FActiveGameplayEffectHandle> HandlesToRemove = GrantedEffectHandles;

	// 리스트 초기화
	GrantedEffectHandles.Empty();

	return HandlesToRemove;
}

void ACMWeaponBase::OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 무기를 소유한 캐릭터를 가져옴 (보통 무기를 들고 있는 플레이어)
	APawn* WeaponOwningPawn = GetInstigator<APawn>();

	checkf(WeaponOwningPawn, TEXT("무기의 소유 폰을 instigator로 설정하는 걸 잊었습니다: %s"), *GetName());

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (HitPawn == WeaponOwningPawn)
		{
			return; // 자기 자신이므로 무시
		}

		// 아군 공격 무시 옵션이 활성화된 경우 적대적인 대상만 공격
		if (bIgnoreFriendly)
		{
			if (UCMFunctionLibrary::IsTargetPawnHostile(WeaponOwningPawn, HitPawn))
			{
				OnWeaponHitTarget.ExecuteIfBound(OtherActor);
			}
		}
		else
		{
			// 아군 공격 무시 비활성화 시 모든 대상 공격
			OnWeaponHitTarget.ExecuteIfBound(OtherActor);
		}
	}
}

void ACMWeaponBase::OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* WeaponOwningPawn = GetInstigator<APawn>();

	checkf(WeaponOwningPawn, TEXT("무기의 소유 폰을 instigator로 설정하는 걸 잊었습니다: %s"), *GetName());

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (HitPawn == WeaponOwningPawn)
		{
			return; // 자기 자신이므로 무시
		}

		// 아군 공격 무시 옵션이 활성화된 경우 적대적인 대상만 처리
		if (bIgnoreFriendly)
		{
			if (UCMFunctionLibrary::IsTargetPawnHostile(WeaponOwningPawn, HitPawn))
			{
				OnWeaponPulledFromTarget.ExecuteIfBound(OtherActor);
			}
		}
		else
		{
			// 아군 공격 무시 비활성화 시 모든 대상 처리
			OnWeaponPulledFromTarget.ExecuteIfBound(OtherActor);
		}
	}
}

void ACMWeaponBase::HideWeapon()
{
	if (WeaponMesh)
	{
		WeaponMesh->SetVisibility(false);
	}

	// 모든 Niagara 컴포넌트 비활성화
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

void ACMWeaponBase::ShowWeapon()
{
	if (WeaponMesh)
	{
		WeaponMesh->SetVisibility(true);
	}

	// 모든 Niagara 컴포넌트 활성화
	TArray<UNiagaraComponent*> NiagaraComponents;
	GetComponents<UNiagaraComponent>(NiagaraComponents);
	for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
	{
		if (NiagaraComp)
		{
			NiagaraComp->Activate();
		}
	}
}