// Copyright CrimsonMoon Team. All Rights Reserved.

#include "GameplayAbilitySystem/Abilities/Player/Arcanist/CMPlayerAbility_SoulSiphon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CMFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CMGameplayTags.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UCMPlayerAbility_SoulSiphon::UCMPlayerAbility_SoulSiphon()
{
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Skill_Arcanist_R);
	SetAssetTags(TagsToAdd);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UCMPlayerAbility_SoulSiphon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!SkillMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 커밋에 실패하면 어빌리티 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (SoundEffectClass)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// GE Spec 생성
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SoundEffectClass, GetAbilityLevel(), ContextHandle);

			if (SpecHandle.IsValid())
			{
				// GE 적용 및 핸들 저장 (EndAbility 시 초기화)
				SoundEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		// 첫 번째 데미지 이벤트 대기
		UAbilityTask_WaitGameplayEvent* WaitFirstDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			CMGameplayTags::Player_Event_SoulSiphon_FirstDamage
			);
		WaitFirstDamageTask->EventReceived.AddDynamic(this, &ThisClass::OnFirstDamageTriggered);
		WaitFirstDamageTask->ReadyForActivation();

		// 두 번째 데미지 이벤트 대기
		UAbilityTask_WaitGameplayEvent* WaitSecondDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			CMGameplayTags::Player_Event_SoulSiphon_SecondDamage
			);
		WaitSecondDamageTask->EventReceived.AddDynamic(this, &ThisClass::OnSecondDamageTriggered);
		WaitSecondDamageTask->ReadyForActivation();
	}

	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SkillMontage,
		1.0f
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UCMPlayerAbility_SoulSiphon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (SoundEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			// 이펙트 제거
			ASC->RemoveActiveGameplayEffect(SoundEffectHandle);
		}

		// 핸들 초기화
		SoundEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMPlayerAbility_SoulSiphon::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_SoulSiphon::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCMPlayerAbility_SoulSiphon::OnFirstDamageTriggered(FGameplayEventData Payload)
{
	ApplyAoEDamageAndSlow(true);
}

void UCMPlayerAbility_SoulSiphon::OnSecondDamageTriggered(FGameplayEventData Payload)
{
	ApplyAoEDamageAndSlow(false);
}

void UCMPlayerAbility_SoulSiphon::ApplyAoEDamageAndSlow(bool bIsFirstDamage)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	ACMPlayerCharacterBase* OwnerCharacter = GetCMPlayerCharacterFromActorInfo();
	if (!OwnerCharacter)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	// 캐릭터 중심으로 스피어 범위 내의 적 탐색
	FVector CenterLocation = OwnerCharacter->GetActorLocation();

	// VFX용 바닥 위치 계산 (캡슐 컴포넌트 바닥)
	FVector BottomLocation = CenterLocation;
	BottomLocation.Z -= OwnerCharacter->GetSimpleCollisionHalfHeight() * 2;

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(OwnerCharacter);
	TArray<AActor*> OverlappedActors;

	// ObjectType 설정 (Pawn만 감지)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// 스피어 오버랩으로 범위 내 모든 액터 탐색
	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		CenterLocation,
		SkillRadius,
		ObjectTypes,
		nullptr,
		IgnoredActors,
		OverlappedActors
		);

	if (bHit)
	{
		for (AActor* HitActor : OverlappedActors)
		{
			if (!HitActor)
			{
				continue;
			}

			// 타겟의 ASC 가져오기
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
			if (!TargetASC)
			{
				continue;
			}

			// 팀 체크
			APawn* SourcePawn = Cast<APawn>(OwnerCharacter);
			APawn* TargetPawn = Cast<APawn>(HitActor);
			if (UCMFunctionLibrary::IsTargetPawnHostile(SourcePawn, TargetPawn))
			{
				// 데미지 적용
				if (DamageEffect)
				{
					UPlayerCombatComponent* PlayerCombatComponent = OwnerCharacter->GetPawnCombatComponent();
					const float BaseDamage = PlayerCombatComponent->GetPlayerCurrentEquippedWeaponDamageAtLevel(GetAbilityLevel()) * DamageMultiplier;
					const float BaseGroggyDamage = PlayerCombatComponent->GetPlayerCurrentWeaponHeavyGroggyDamage(GetAbilityLevel());

					FGameplayEffectSpecHandle DamageSpecHandle = MakeDamageEffectSpecHandle(
						DamageEffect,
						BaseDamage,
						BaseGroggyDamage,
						CMGameplayTags::Player_SetByCaller_AttackType_Light,
						0
						);

					if (DamageSpecHandle.IsValid())
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
					}
				}

				// 슬로우 효과 적용 (첫 번째 데미지에만)
				if (bIsFirstDamage && SlowEffect)
				{
					FGameplayEffectSpecHandle SlowSpecHandle = SourceASC->MakeOutgoingSpec(
						SlowEffect,
						GetAbilityLevel(),
						SourceASC->MakeEffectContext()
						);

					if (SlowSpecHandle.IsValid())
					{
						// 슬로우 강도와 지속시간 설정
						SlowSpecHandle.Data->SetSetByCallerMagnitude(
							CMGameplayTags::SetByCaller_SlowIntensity,
							SlowIntensity
							);

						SlowSpecHandle.Data->SetDuration(SlowDuration, true);

						TargetASC->ApplyGameplayEffectSpecToSelf(*SlowSpecHandle.Data.Get());
					}
				}

				// GameplayCue 실행 (피격 효과)
				ExecuteTargetHitGameplayCue(HitActor, CMGameplayTags::GameplayCue_Arcanist_SoulSiphon_Hit);
			}
		}
	}

	// VFX 실행 (캐릭터 바닥, 첫 번째 데미지에만)
	if (bIsFirstDamage)
	{
		FGameplayCueParameters VFXCueParams;
		VFXCueParams.Location = BottomLocation;
		VFXCueParams.Instigator = OwnerCharacter;
		VFXCueParams.EffectCauser = OwnerCharacter;

		SourceASC->ExecuteGameplayCue(
			CMGameplayTags::GameplayCue_Arcanist_SoulSiphon_VFX,
			VFXCueParams
			);
	}
}