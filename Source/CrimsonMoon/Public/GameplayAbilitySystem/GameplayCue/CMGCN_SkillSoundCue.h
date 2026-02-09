// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "CMGCN_SkillSoundCue.generated.h"

/**
 * 
 */
UCLASS()
class CRIMSONMOON_API ACMGCN_SkillSoundCue : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	ACMGCN_SkillSoundCue();

protected:
	// 큐가 활성화될 때 (태그 부착 시)
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// 큐가 제거될 때 (태그 제거 시 - 스킬 취소/종료)
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// Fade Out이 끝난 후 액터를 정리하기 위한 함수
	void FinishFadeOut();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<UAudioComponent> AudioComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* LoopingSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	float FadeOutDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	float FadeInDuration = 0.1f;

	FTimerHandle FadeOutTimerHandle;
};