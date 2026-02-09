// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CMAnimInstanceBase.generated.h"

class UAbilitySystemComponent;
struct FGameplayTag;
class ACMCharacterBase;
class UCharacterMovementComponent;
/**
 * @class UCMAnimInstanceBase
 * @brief 애니메이션 인스턴스의 기본 클래스.
 *
 * UAnimInstance를 상속받은 Crimson Moon 프로젝트의 기본 애니메이션 인스턴스 클래스입니다.
 * 이 클래스는 캐릭터의 상태 기반 애니메이션 기능을 확장하거나 커스터마이즈하는 데 사용됩니다.
 * UCMPlayerAnimInstance 및 UCMPlayerLinkedAnimLayer와 같은 플레이어 관련 애니메이션 클래스의 기본 클래스 역할을 합니다.
 */
UCLASS()
class CRIMSONMOON_API UCMAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsJumping() const { return IsFalling; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsOnGround() const { return !IsFalling; }

protected:
	UFUNCTION(BlueprintPure, Category = "AnimData|Character", meta = (BlueprintThreadSafe))
	bool DoesOwnerHaveTag(FGameplayTag TagToCheck) const;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Character")
	TObjectPtr<ACMCharacterBase> OwningCharacter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Character")
	TObjectPtr<UCharacterMovementComponent> OwningMovementComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Locomotion")
	float GroundSpeed = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Locomotion")
	bool IsFalling = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Locomotion")
	bool bHasAcceleration = false;
};