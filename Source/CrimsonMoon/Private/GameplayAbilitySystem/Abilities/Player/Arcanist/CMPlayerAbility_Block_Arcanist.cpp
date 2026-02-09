// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/Arcanist/CMPlayerAbility_Block_Arcanist.h"

#include "Character/Player/CMPlayerCharacterBase.h"

void UCMPlayerAbility_Block_Arcanist::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 부모 클래스가 이미 OnMontageEnded를 바인딩했으므로,
	// 자식 클래스에서는 OnMontageEnded를 오버라이드하여 어빌리티 종료 처리

	// 1. 배리어 소환
	if (HasAuthority(&ActivationInfo) && BarrierActorClass)
	{
		if (ACMPlayerCharacterBase* PlayerCharacterBase = Cast<ACMPlayerCharacterBase>(GetAvatarActorFromActorInfo()))
		{
			const FVector SpawnLocation = PlayerCharacterBase->GetActorLocation() + (PlayerCharacterBase->GetActorForwardVector() * BarrierSpawnOffset.X) + (PlayerCharacterBase->GetActorRightVector() * BarrierSpawnOffset.Y) + (PlayerCharacterBase->GetActorUpVector() * BarrierSpawnOffset.Z);
			const FRotator SpawnRotation = PlayerCharacterBase->GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = PlayerCharacterBase;
			SpawnParams.Instigator = PlayerCharacterBase;

			SpawnedBarrier = GetWorld()->SpawnActor<AActor>(BarrierActorClass, SpawnLocation, SpawnRotation, SpawnParams);

			// 필요하다면 배리어를 캐릭터에 부착 (이동 시 따라다니게 하려면)
			if (SpawnedBarrier)
			{
				SpawnedBarrier->AttachToActor(PlayerCharacterBase, FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}
}

void UCMPlayerAbility_Block_Arcanist::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 서버에서만 배리어 파괴 (리플리케이션으로 클라이언트에도 자동 적용)
	if (HasAuthority(&ActivationInfo) && SpawnedBarrier)
	{
		SpawnedBarrier->Destroy();
		SpawnedBarrier = nullptr;
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMPlayerAbility_Block_Arcanist::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// 아케이니스트는 입력 해제로 종료하지 않음 (몽타주 종료 시 자동 종료)
	// 부모 클래스의 InputReleased를 호출하지 않음
}

void UCMPlayerAbility_Block_Arcanist::OnPerfectParrySuccess(FGameplayEventData Payload)
{
	// 서버에서만 실행 (리슨 서버 환경 고려)
	if (!GetOwningActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	// 아케이니스트는 카운터 어택이 불가능하므로 아무것도 하지 않음
	// 퍼펙트 블록 시 데미지만 완전히 막고, 카운터 어택 윈도우나 그로기 데미지를 부여하지 않음
}

void UCMPlayerAbility_Block_Arcanist::OnMontageEnded()
{
	// 몽타주가 끝나면 어빌리티 종료 (Player.Status.Blocking 태그 자동 제거됨)
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}