// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbilitySystem/Abilities/Common/CMAbility_Death.h"
#include "CMGameplayTags.h"
#include "Character/CMCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/CMWeaponBase.h"
#include "TimerManager.h"
#include "AIController.h"
#include "Animation/AnimMontage.h"
#include "Controllers/CMPlayerController.h"

UCMAbility_Death::UCMAbility_Death()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Shared_Ability_Death);
	SetAssetTags(TagsToAdd);

	// 서버에서만 실행 (몽타주는 캐릭터의 NetMulticast RPC로 모든 클라이언트에 전파)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
}

void UCMAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACMCharacterBase* CharacterBase = GetCMCharacterFromActorInfo();
	if (!CharacterBase)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 1. CharacterMovement 비활성화
	if (UCharacterMovementComponent* MovementComp = CharacterBase->GetCharacterMovement())
	{
		MovementComp->DisableMovement();
		MovementComp->StopMovementImmediately();
	}

	// 2. 플레이어 입력 비활성화
	if (APlayerController* PC = Cast<APlayerController>(CharacterBase->GetController()))
	{
		CharacterBase->DisableInput(PC);
	}

	// 3. 서버 전용 로직 실행
	HandleDeathOnServer();

	// 4. 몽타주 랜덤 선택
	UAnimMontage* SelectedMontage = nullptr;
	if (DeathMontages.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandRange(0, DeathMontages.Num() - 1);
		SelectedMontage = DeathMontages[RandomIndex];
	}

	if (SelectedMontage)
	{
		// 캐릭터의 NetMulticast RPC를 호출하여 모든 클라이언트에서 몽타주 재생
		CharacterBase->Multicast_PlayDeathMontage(SelectedMontage);

		const float MontageLength = SelectedMontage->GetPlayLength();

		// 파괴 타이머 설정 (몽타주 길이 + 딜레이)
		FTimerDelegate DestroyDelegate;
		DestroyDelegate.BindUObject(this, &ThisClass::HandleCharacterDestruction);

		GetWorld()->GetTimerManager().SetTimer(
			DestroyTimerHandle,
			DestroyDelegate,
			MontageLength + DestroyDelay,
			false
			);

		// 몽타주 길이만큼 대기 후 어빌리티 종료
		FTimerDelegate EndAbilityDelegate;
		EndAbilityDelegate.BindWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo]() {
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		});

		FTimerHandle EndAbilityTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			EndAbilityTimerHandle,
			EndAbilityDelegate,
			MontageLength,
			false
			);
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UCMAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 어빌리티가 취소된 경우에만 타이머 정리 (정상 종료 시에는 캐릭터 파괴 진행)
	if (bWasCancelled && DestroyTimerHandle.IsValid())
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DestroyTimerHandle);
			DestroyTimerHandle.Invalidate();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMAbility_Death::HandleDeathOnServer()
{
	const ACMCharacterBase* CharacterBase = GetCMCharacterFromActorInfo();
	if (!CharacterBase)
	{
		return;
	}

	// 1. AI 컨트롤러 비활성화 (AI인 경우) - 서버에서만 처리
	if (AAIController* AI = Cast<AAIController>(CharacterBase->GetController()))
	{
		// AI->GetBrainComponent()->StopLogic(TEXT("Dead"));
		AI->UnPossess();
	}

	// 2. 콜리전 설정 변경 - 서버에서만 처리 (리플리케이션됨)
	if (UCapsuleComponent* CapsuleComp = CharacterBase->GetCapsuleComponent())
	{
		// 폰 콜리전 비활성화 (더 이상 밀리지 않음)
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// 3. 메시 콜리전 설정 - 서버에서만 처리 (리플리케이션됨)
	if (USkeletalMeshComponent* MeshComp = CharacterBase->GetMesh())
	{
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	// 4. ASC의 모든 활성 어빌리티 취소 (사망 어빌리티 제외) - 서버에서만 처리
	if (UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo())
	{
		ASC->CancelAbilities(nullptr, &ActivationOwnedTags);
	}

	// 5. 현재 장착된 무기 액터 파괴
	if (const UPawnCombatComponent* CombatComponent = GetPawnCombatComponentFromActorInfo())
	{
		if (ACMWeaponBase* EquippedWeapon = CombatComponent->GetCharacterCurrentEquippedWeapon())
		{
			EquippedWeapon->Destroy();
		}
	}

	// 6. 서버에서 사망 처리: 플레이어가 조종하는 Pawn이면 PlayerController에 알림
	if (AController* Controller = CharacterBase->GetController())
	{
		// AIController가 아니라 PlayerController인지 확인
		if (ACMPlayerController* CMPC = Cast<ACMPlayerController>(Controller))
		{
			// 서버에서만 동작하는 안전장치가 Controller 쪽에도 있으나, 여기서도 한 번 더 보장
			CMPC->HandlePlayerDeath();
		}
	}
}

void UCMAbility_Death::HandleDeathCosmetics()
{
	// TODO: 모든 클라이언트에서 실행되는 시각적 효과 추가 예정
	// GameplayCue를 통해 파티클 효과, 사운드 재생 등 처리
}

void UCMAbility_Death::HandleCharacterDestruction()
{
	// 타이머 핸들 먼저 무효화 (재진입 방지)
	DestroyTimerHandle.Invalidate();

	// 어빌리티와 액터 유효성 검사
	if (!IsValid(this) || !CurrentActorInfo)
	{
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!IsValid(Avatar))
	{
		return;
	}

	// 서버에서만 실행되어야 함
	if (HasAuthority(&CurrentActivationInfo))
	{
		Avatar->Destroy();
	}
}