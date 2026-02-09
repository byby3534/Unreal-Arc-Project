// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/Arcanist/CMBarrierActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CMGameplayTags.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GameplayAbilitySystem/Abilities/Skill/Projectile/CMProjectileActor.h"


ACMBarrierActor::ACMBarrierActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 리플리케이션 설정 (리슨 서버 환경 지원)
	bReplicates = true;
	SetReplicateMovement(true);

	// 충돌체(Sphere Collision) 생성 및 설정
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);

	SphereCollision->SetSphereRadius(150.f); // 반지름 설정
	SphereCollision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	SphereCollision->SetSimulatePhysics(false);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 나이아가라 컴포넌트 생성 및 설정
	BarrierEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BarrierEffectComponent"));
	BarrierEffectComponent->SetupAttachment(RootComponent);

	// 생성되자마자 이펙트가 보이도록 설정
	BarrierEffectComponent->bAutoActivate = true;

	// 배리어 크기에 맞춰 이펙트가 따라가거나 회전하도록 설정 (필요 시 변경)
	BarrierEffectComponent->SetRelativeLocation(FVector::ZeroVector);
}

void ACMBarrierActor::BeginPlay()
{
	Super::BeginPlay();

	// 충돌 이벤트 바인딩
	if (SphereCollision)
	{
		SphereCollision->OnComponentHit.AddDynamic(this, &ACMBarrierActor::OnBarrierHit);

		// Owner(플레이어)와의 충돌 무시
		if (AActor* OwnerActor = GetOwner())
		{
			SphereCollision->IgnoreActorWhenMoving(OwnerActor, true);
			// Owner의 모든 컴포넌트와 충돌 무시
			SphereCollision->MoveIgnoreActors.Add(OwnerActor);
		}

		// 배리어 시각화 설정
		if (bVisualizeBarrier)
		{
			#if WITH_EDITOR
			DrawDebugSphere(
				GetWorld(),
				SphereCollision->GetComponentLocation(),
				SphereCollision->GetScaledSphereRadius(),
				24,
				FColor::Green,
				false,
				5.0f,
				0,
				2.0f
				);
			#endif
		}
	}
}

void ACMBarrierActor::OnBarrierHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	// 투사체인 경우 파괴하고 블록 이벤트 발송
	if (ACMProjectileActor* Projectile = Cast<ACMProjectileActor>(OtherActor))
	{
		// 서버에서만 처리
		if (HasAuthority())
		{
			// 블록 이벤트 발송
			SendBlockEventToOwner(OtherActor);

			// 투사체 파괴
			Projectile->Destroy();
		}
	}
	// 다른 액터의 경우 기존 블록 시스템이 처리 (AttributeSet의 TryBlockOrParry)
}

void ACMBarrierActor::SendBlockEventToOwner(AActor* Attacker) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// Owner의 ASC 가져오기
	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerActor);
	if (!ASI)
	{
		return;
	}

	const UAbilitySystemComponent* OwnerASC = ASI->GetAbilitySystemComponent();
	if (!OwnerASC)
	{
		return;
	}

	// 이벤트 데이터 생성
	FGameplayEventData Payload;
	Payload.Target = OwnerActor;
	Payload.Instigator = Attacker;

	// PerfectParryWindow 태그 확인
	const bool bIsPerfectParry = OwnerASC->HasMatchingGameplayTag(CMGameplayTags::Player_Status_PerfectParryWindow);

	if (bIsPerfectParry)
	{
		// 퍼펙트 블록 성공 이벤트 발송
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, CMGameplayTags::Player_Event_PerfectParrySuccess, Payload);
	}

	// 블록 성공 이벤트 발송 (퍼펙트든 일반이든 모두 발송)
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, CMGameplayTags::Player_Event_SuccessfulBlock, Payload);
}