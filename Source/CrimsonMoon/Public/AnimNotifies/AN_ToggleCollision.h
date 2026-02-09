// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Enums/CMEnums.h"
#include "AN_ToggleCollision.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API UAN_ToggleCollision : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	/* 몽타주에서 설정: 어떤 타입의 콜리전을 켤 것인지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	EToggleDamageType DamageTypeToToggle = EToggleDamageType::CurrentEquippedWeapon;
};
