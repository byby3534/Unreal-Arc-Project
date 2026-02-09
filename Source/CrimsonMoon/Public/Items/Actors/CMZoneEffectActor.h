// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Enums/CMEnums.h"
#include "CMZoneEffectActor.generated.h"

class USphereComponent;
class UGameplayEffect;
class UAbilitySystemComponent;

// 핸들과 ASC를 함께 저장하는 구조체
struct FZoneAppliedEffectData
{
	FActiveGameplayEffectHandle Handle;
	TWeakObjectPtr<UAbilitySystemComponent> TargetASC;
};

/*
 * 땅에 설치되는 액터
 */
UCLASS()
class CRIMSONMOON_API ACMZoneEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACMZoneEffectActor();

	// 수명 설정 함수
	void InitializeDuration(float InDuration);

	// 메쉬 설정 함수
	void SetMesh(UStaticMesh* InMesh);

	// 이펙트 설정 함수
	void SetZoneGameplayEffect(TSubclassOf<UGameplayEffect> InEffect);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

	// 범위 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> AreaSphere;

	// 시각적 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// [GAS 설정] 적용할 게임플레이 이펙트 (치유, 버프, 도트 데미지 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> ZoneGameplayEffect;

	// 적용할 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	EZoneTargetPolicy TargetPolicy = EZoneTargetPolicy::All;

	// [오버랩 이벤트 함수]
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 대상 유효성 검사 함수 (아군/적군 판별)
	bool IsValidTarget(AActor* TargetActor) const;

	// 적용된 이펙트 관리
	TMap<AActor*, FZoneAppliedEffectData> ActiveEffectMap;

	// 미리 만들어둔 이펙트 스펙을 저장
	FGameplayEffectSpecHandle CachedZoneEffectSpec;

	// 시전자의 팀 태그 저장
	FGameplayTag CachedInstigatorTeamTag;
};
