// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/GameplayCue/CMGCN_SkillSoundCue.h"

#include "Components/AudioComponent.h"

ACMGCN_SkillSoundCue::ACMGCN_SkillSoundCue()
{
	PrimaryActorTick.bCanEverTick = false;
	bAutoDestroyOnRemove = false;

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	RootComponent = AudioComp;
	AudioComp->bAutoActivate = false; // 수동 제어
}

bool ACMGCN_SkillSoundCue::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);

	if (LoopingSound)
	{
		AudioComp->SetSound(LoopingSound);
		AudioComp->FadeIn(FadeInDuration);
	}

	return true;
}

bool ACMGCN_SkillSoundCue::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnRemove_Implementation(MyTarget, Parameters);

	if (AudioComp && AudioComp->IsPlaying())
	{
		AudioComp->FadeOut(FadeOutDuration, 0.0f);

		// Fade Out 시간만큼 기다린 후 액터를 파괴하기 위해 타이머 설정
		GetWorld()->GetTimerManager().SetTimer(
			FadeOutTimerHandle,
			this,
			&ACMGCN_SkillSoundCue::FinishFadeOut,
			FadeOutDuration,
			false
			);
	}
	else
	{
		// 재생 중이 아니었다면 바로 파괴
		FinishFadeOut();
	}

	return true;
}

void ACMGCN_SkillSoundCue::FinishFadeOut()
{
	// 액터 스스로 파괴
	Destroy();
}