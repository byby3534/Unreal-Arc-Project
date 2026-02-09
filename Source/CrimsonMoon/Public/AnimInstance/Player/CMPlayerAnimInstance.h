// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimInstance/CMAnimInstanceBase.h"
#include "CMPlayerAnimInstance.generated.h"

class ACMPlayerCharacterBase;
/**
 * @class UCMPlayerAnimInstance
 * @brief 플레이어 캐릭터의 애니메이션 로직을 처리하는 애니메이션 인스턴스 클래스.
 *
 * UCMAnimInstanceBase를 상속받아 구현되며, Crimson Moon 프로젝트의 플레이어 캐릭터와 관련된 애니메이션 로직을
 * 정의하는 데 사용됩니다. 각종 상태 및 동작과 관련된 애니메이션 데이터를 관리하거나, 이를 기반으로
 * 애니메이션 블루프린트 내의 다양한 동작을 처리할 수 있습니다.
 */
UCLASS()
class CRIMSONMOON_API UCMPlayerAnimInstance : public UCMAnimInstanceBase
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	// Turn in Place 업데이트 함수
	void UpdateTurnInPlace(float DeltaSeconds);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Character")
	TObjectPtr<ACMPlayerCharacterBase> OwningPlayerCharacter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Locomotion")
	bool IsCrouching = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Locomotion")
	bool IsSprinting = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Locomotion")
	float MoveDirection = 0.f;

	// Turn in Place 관련 변수
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|TurnInPlace")
	float YawDelta = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|TurnInPlace")
	float RootYawOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimData|TurnInPlace")
	float TurnCheckMinThreshold = 45.f; // 회전 시작 임계값

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimData|TurnInPlace")
	float YawDeltaInterpSpeed = 10.f; // 보간 속도

private:
	FRotator LastActorRotation = FRotator::ZeroRotator;
};