// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "Engine/DataAsset.h"
#include "Structs/CMStructTypes.h"
#include "CMDataAsset_ConsumableData.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;
class UStaticMesh;
class ACMZoneEffectActor;

UENUM(BlueprintType)
enum class EConsumableType : uint8
{
	Immediate   UMETA(DisplayName = "Immediate|즉발형"),
	Throwable   UMETA(DisplayName = "Throwable|투척형"),
	Placeable   UMETA(DisplayName = "Placeable|설치형")
};

// 즉발형 아이템 (포션, 음식 등)
USTRUCT(BlueprintType)
struct FCMImmediateData
{
	GENERATED_BODY()

	// 사용 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> UseMontage;
	
};

// 투척형 아이템
USTRUCT(BlueprintType)
struct FCMThrowableData
{
	GENERATED_BODY()

	// 투척 시 스폰될 발사체(Projectile) 액터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> ProjectileClass;

	// 발사체가 생성될 캐릭터의 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName ThrowSocketName = FName("Hand_R_Socket");

	// 투척 속도 (Force)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ThrowSpeed = 1000.0f;

	// 중력 영향 (1.0 = 일반, 0.0 = 직사)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GravityScale = 1.0f;

	// 투척 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> ThrowMontage;

	// 몽타주 재생 후 발사까지의 시간 (딜레이)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = "0.0", ClampMin = "0.0"))
	float ThrowDelay = 0.3f;
};

// 설치형 아이템
USTRUCT(BlueprintType)
struct FCMPlaceableData
{
	GENERATED_BODY()

	// 설치될 실제 액터 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ACMZoneEffectActor> PlaceableActorClass;

	// 설치될 액터의 스태틱 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> PlaceableMesh;

	// 장판 지속 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Duration = 10.0f;

	// 바닥 경사에 맞춰 회전할지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bSnapToGround = true;
};

/**
 * 소비, 투척, 설치 아이템 통합 데이터 에셋
 */
UCLASS()
class CRIMSONMOON_API UCMDataAsset_ConsumableData : public UCMDataAsset_ItemBase
{
	GENERATED_BODY()

public:
	// 이 아이템의 작동 방식 선택
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type")
	EConsumableType ConsumableType = EConsumableType::Immediate;
	
	// 캐릭터 손에 들고 있을 때 보여줄 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visual")
	TObjectPtr<UStaticMesh> HandHeldMesh;

	// 캐릭터 손에 들 때 부착할 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visual")
	FName HandHeldSocketName = FName("Hand_Item");

	// 캐릭터 손에 들 때 메쉬 크기 조절
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visual")
	FVector HandHeldMeshScale = FVector(1.0f);

	// 사용 시 실행할 어빌리티
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> ActivationAbility;

	// 사용 시 적용할 효과 (즉시 회복 등. 투척/설치는 발사체/설치물에서 처리할 예정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> ConsumableEffect;

	// 장착 시 사용할 애니메이션 몽타주 - 꺼내는 동작
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UAnimMontage> EquipMontage;

	// [즉발형 데이터] ConsumableType이 Immediate일 때만 표시됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Data", meta = (EditCondition = "ConsumableType == EConsumableType::Immediate", EditConditionHides))
	FCMImmediateData ImmediateData;
	
	// [투척형 데이터] ConsumableType이 Throwable일 때만 표시됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Data", meta = (EditCondition = "ConsumableType == EConsumableType::Throwable", EditConditionHides))
	FCMThrowableData ThrowableData;

	// [설치형 데이터] ConsumableType이 Placeable일 때만 표시됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Data", meta = (EditCondition = "ConsumableType == EConsumableType::Placeable", EditConditionHides))
	FCMPlaceableData PlaceableData;
	
};
