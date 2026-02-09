// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Actors/CMZoneEffectActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameplayTags/CMGameplayTags_Character.h"
#include "GameplayTags/CMGameplayTags_Enemy.h"
#include "Enums/CMEnums.h"

ACMZoneEffectActor::ACMZoneEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetSphereRadius(300.0f);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 폰만 감지
}

void ACMZoneEffectActor::InitializeDuration(float InDuration)
{
	// 0보다 클 때만 수명을 설정 (0이면 영구 지속)
	if (InDuration > 0.0f)
	{
		SetLifeSpan(InDuration);
	}
	else
	{
		SetLifeSpan(0.0f);
	}
}

void ACMZoneEffectActor::SetMesh(UStaticMesh* InMesh)
{
	if (MeshComponent && InMesh)
	{
		MeshComponent->SetStaticMesh(InMesh);
	}
}

void ACMZoneEffectActor::SetZoneGameplayEffect(TSubclassOf<UGameplayEffect> InEffect)
{
	ZoneGameplayEffect = InEffect;
}

void ACMZoneEffectActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (GetInstigator())
		{
			UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
            
			if (InstigatorASC)
			{
				// [팀 태그 캐싱] 시전자가 Player인지 Enemy인지 저장
				if (InstigatorASC->HasMatchingGameplayTag(CMGameplayTags::Character_Class_Common))
				{
					CachedInstigatorTeamTag = CMGameplayTags::Character_Class_Common;
				}
				else if (InstigatorASC->HasMatchingGameplayTag(CMGameplayTags::Enemy_Type_Common))
				{
					CachedInstigatorTeamTag = CMGameplayTags::Enemy_Type_Common;
				}

				// [이펙트 스펙 미리 생성] 
				if (ZoneGameplayEffect)
				{
					FGameplayEffectContextHandle ContextHandle = InstigatorASC->MakeEffectContext();
					ContextHandle.AddSourceObject(this);

					CachedZoneEffectSpec = InstigatorASC->MakeOutgoingSpec(ZoneGameplayEffect, 1.0f, ContextHandle);
				}
			}
		}

		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ACMZoneEffectActor::OnOverlapBegin);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ACMZoneEffectActor::OnOverlapEnd);
	}
}

void ACMZoneEffectActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//  이펙트 제거
	if (HasAuthority())
	{
	
		for (auto& Elem : ActiveEffectMap)
		{
			
			FZoneAppliedEffectData& Data = Elem.Value;
			
			if (UAbilitySystemComponent* TargetASC = Data.TargetASC.Get())
			{
				TargetASC->RemoveActiveGameplayEffect(Data.Handle, 1);
			}
		}

		// 맵 비우기
		ActiveEffectMap.Empty();
	}
	
	Super::EndPlay(EndPlayReason);
}


// 대상이 효과를 받을 자격이 있는지 검사
bool ACMZoneEffectActor::IsValidTarget(AActor* TargetActor) const
{
	if (!TargetActor || TargetActor == this)
	{
		return false;
	}
	
	// 대상의 ASC 가져오기
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return false;
	}

	// 정책에 따른 판별
	switch (TargetPolicy)
	{
		case EZoneTargetPolicy::All:
			return true;

		case EZoneTargetPolicy::Friendly:
			// 대상이 시전자와 같은 팀 태그를 가지고 있는지 확인
			if (CachedInstigatorTeamTag.IsValid())
			{
				return TargetASC->HasMatchingGameplayTag(CachedInstigatorTeamTag);
			}
			return true;

		case EZoneTargetPolicy::Enemy:
			// 대상이 시전자와 다른 팀 태그인지 확인
			if (CachedInstigatorTeamTag.IsValid())
			{
				return !TargetASC->HasMatchingGameplayTag(CachedInstigatorTeamTag);
			}
			return true;
	}

	return false;
}

void ACMZoneEffectActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 조건: 서버 권한 + GE 설정됨 + 설치자 ASC 유효 + 이미 적용 안 됨 + 타겟 유효성 통과
	if (!HasAuthority() || !CachedZoneEffectSpec.IsValid())
	{
		return;
	}
	
	if (ActiveEffectMap.Contains(OtherActor))
	{
		return;
	}
	
	if (!IsValidTarget(OtherActor))
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC)
	{
		// 미리 만들어둔 스펙을 대상에게 적용
		FActiveGameplayEffectHandle Handle = TargetASC->ApplyGameplayEffectSpecToSelf(*CachedZoneEffectSpec.Data.Get());

		// 핸들과 ASC를 구조체로 묶어서 저장
		FZoneAppliedEffectData EffectData;
		EffectData.Handle = Handle;
		EffectData.TargetASC = TargetASC;

		ActiveEffectMap.Add(OtherActor, EffectData);
	}
}

void ACMZoneEffectActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	// 목록에 있다면 제거
	if (FZoneAppliedEffectData* EffectData = ActiveEffectMap.Find(OtherActor))
	{
		// 저장해두었던 ASC가 여전히 유효한지 확인
		UAbilitySystemComponent* TargetASC = EffectData->TargetASC.Get();

		// ASC가 유효하다면 이펙트를 제거
		if (TargetASC)
		{
			TargetASC->RemoveActiveGameplayEffect(EffectData->Handle, 1);
		}

		// 맵에서 안전히 제거
		ActiveEffectMap.Remove(OtherActor);
	}
}
