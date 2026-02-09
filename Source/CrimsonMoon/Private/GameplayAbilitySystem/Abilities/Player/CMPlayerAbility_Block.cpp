// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/CMPlayerAbility_Block.h"

#include "CMGameplayTags.h"
#include "CMFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"

UCMPlayerAbility_Block::UCMPlayerAbility_Block()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Block);
	SetAssetTags(TagsToAdd);

	ActivationOwnedTags.AddTag(CMGameplayTags::Player_Status_Blocking);
	ActivationBlockedTags.AddTag(CMGameplayTags::Player_Status_Blocking);
	ActivationBlockedTags.AddTag(CMGameplayTags::Shared_Status_Dead);

	ParryWindowStartEventTag = CMGameplayTags::Player_Event_ParryWindowStart;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UCMPlayerAbility_Block::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 커밋에 실패하면 (예: 스태미나 부족) 어빌리티를 즉시 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (BlockStartMontage)
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, BlockStartMontage);

		// 몽타주가 끝나도 어빌리티가 종료되지 않도록 OnMontageEnded를 바인딩
		MontageTask->OnCompleted.AddDynamic(this, &UCMPlayerAbility_Block::OnMontageEnded);
		MontageTask->OnBlendOut.AddDynamic(this, &UCMPlayerAbility_Block::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UCMPlayerAbility_Block::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UCMPlayerAbility_Block::OnMontageEnded);

		MontageTask->ReadyForActivation();
	}

	if (ParryWindowStartEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* WaitParryEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ParryWindowStartEventTag);
		WaitParryEventTask->EventReceived.AddDynamic(this, &ThisClass::OnParryWindowStart);
		WaitParryEventTask->ReadyForActivation();
	}

	UAbilityTask_WaitGameplayEvent* WaitParrySuccessEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CMGameplayTags::Player_Event_SuccessfulBlock);
	WaitParrySuccessEventTask->EventReceived.AddDynamic(this, &ThisClass::OnSuccessfulParry);
	WaitParrySuccessEventTask->ReadyForActivation();

	// 퍼펙트 패링 성공 이벤트 대기 (AttributeSet에서 발송)
	UAbilityTask_WaitGameplayEvent* WaitPerfectParrySuccessTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CMGameplayTags::Player_Event_PerfectParrySuccess);
	WaitPerfectParrySuccessTask->EventReceived.AddDynamic(this, &ThisClass::OnPerfectParrySuccess);
	WaitPerfectParrySuccessTask->ReadyForActivation();

	// 이 어빌리티는 입력이 해제될 때(InputReleased) EndAbility가 호출될 때까지 활성 상태를 유지
}

void UCMPlayerAbility_Block::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// 방어 버튼을 떼면 어빌리티 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_Block::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo())
	{
		// 패링 GE 제거
		if (ParryWindowEffectHandle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(ParryWindowEffectHandle);
		}
	}

	ParryWindowEffectHandle.Invalidate(); // 핸들 무효화

	if (MontageTask && MontageTask->IsActive())
	{
		MontageTask->EndTask();
	}

	MontageTask = nullptr; // 참조 제거

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UCMPlayerAbility_Block::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ParryWindowEffect)
	{
		return false;
	}

	return true;
}

void UCMPlayerAbility_Block::OnMontageEnded()
{
	if (MontageTask)
	{
		MontageTask = nullptr;
	}
}

void UCMPlayerAbility_Block::OnParryWindowStart(FGameplayEventData Payload)
{
	UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo();

	// 이펙트가 유효하고, 아직 적용되지 않았을 때만 적용
	if (AbilitySystemComponent && ParryWindowEffect && !ParryWindowEffectHandle.IsValid())
	{
		ParryWindowEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(ParryWindowEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, AbilitySystemComponent->MakeEffectContext());
	}
}

void UCMPlayerAbility_Block::OnSuccessfulParry(FGameplayEventData Payload)
{
	K2_ExecuteGameplayCueWithParams(CMGameplayTags::GameplayCue_FX_Blade_SuccessfulBlock, MakeBlockGameplayCueParams());

	// 퍼펙트 블록 판정은 서버에서만 수행 (리슨 서버 환경 고려)
	if (!GetOwningActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	// ASC가 PerfectParryWindow 태그를 가지고 있는지 확인
	UCMAbilitySystemComponent* AbilitySystemComponent = GetCMAbilitySystemComponentFromActorInfo();
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(CMGameplayTags::Player_Status_PerfectParryWindow))
	{
		OnPerfectBlock();
	}
}

void UCMPlayerAbility_Block::OnPerfectBlock()
{
	K2_ExecuteGameplayCueWithParams(CMGameplayTags::GameplayCue_FX_Blade_PerfectBlock, MakeBlockGameplayCueParams());
}

void UCMPlayerAbility_Block::OnPerfectParrySuccess(FGameplayEventData Payload)
{
	// 서버에서만 GameplayEffect 적용 (자동으로 복제됨)
	if (!GetOwningActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	UCMAbilitySystemComponent* DefenderASC = GetCMAbilitySystemComponentFromActorInfo();
	if (!DefenderASC)
	{
		return;
	}

	// 1. 카운터 어택 윈도우 GameplayEffect 적용 (패링 성공한 플레이어에게)
	if (CounterAttackWindowEffect)
	{
		DefenderASC->ApplyGameplayEffectToSelf(
			CounterAttackWindowEffect->GetDefaultObject<UGameplayEffect>(),
			1.0f,
			DefenderASC->MakeEffectContext()
			);
	}

	// 2. 공격자에게 그로기 데미지 적용
	const AActor* Attacker = Payload.Instigator.Get();

	if (Attacker && ParryGroggyDamageEffect)
	{
		// IAbilitySystemInterface를 통해 안전하게 ASC 가져오기
		const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(Attacker);

		if (!AbilitySystemInterface)
		{
			return;
		}

		if (UCMAbilitySystemComponent* AttackerASC = Cast<UCMAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent()))
		{

			FGameplayEffectContextHandle EffectContext = DefenderASC->MakeEffectContext();
			EffectContext.AddInstigator(GetOwningActorFromActorInfo(), GetOwningActorFromActorInfo());
			EffectContext.AddHitResult(FHitResult(), true);

			const FGameplayEffectSpecHandle SpecHandle = DefenderASC->MakeOutgoingSpec(
				ParryGroggyDamageEffect,
				GetAbilityLevel(),
				EffectContext
				);

			if (SpecHandle.IsValid())
			{
				DefenderASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AttackerASC);
			}
		}
	}
}

FGameplayCueParameters UCMPlayerAbility_Block::MakeBlockGameplayCueParams() const
{
	FGameplayCueParameters GameplayCueParams;

	// 플레이어 액터 가져오기
	AActor* OwnerActor = GetOwningActorFromActorInfo();
	if (!OwnerActor)
	{
		return GameplayCueParams;
	}

	// GameplayCue 기본 정보 설정
	GameplayCueParams.Instigator = OwnerActor;
	GameplayCueParams.EffectCauser = OwnerActor;
	GameplayCueParams.Location = OwnerActor->GetActorLocation();

	USkeletalMeshComponent* TargetMesh = UCMFunctionLibrary::FindSkeletalMeshByTag(
		OwnerActor,
		TargetMeshTagName,
		true // 못 찾으면 기본 메시 반환
		);

	GameplayCueParams.TargetAttachComponent = TargetMesh;

	return GameplayCueParams;
}