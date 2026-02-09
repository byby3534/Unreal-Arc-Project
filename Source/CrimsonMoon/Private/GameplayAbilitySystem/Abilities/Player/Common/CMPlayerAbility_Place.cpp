// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_Place.h"
#include "DataAssets/Consumable/CMDataAsset_ConsumableData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "Items/Object/CMItemInstance.h"
#include "Items/Actors/CMZoneEffectActor.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CMGameplayTags.h"

UCMPlayerAbility_Place::UCMPlayerAbility_Place()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Item_Place);

	CancelAbilitiesWithTag.AddTag(CMGameplayTags::Player_Ability_Item_Place);
}

void UCMPlayerAbility_Place::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 아이템 인스턴스 가져오기
	UCMItemInstance* ItemInstance = Cast<UCMItemInstance>(GetSourceObject(Handle, ActorInfo));

	// 인스턴스가 없으면 퀵바에서 찾기
	if (!ItemInstance)
	{
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			if (UCMQuickBarComponent* QuickBar = Avatar->FindComponentByClass<UCMQuickBarComponent>())
			{
				ItemInstance = QuickBar->GetActiveSlotItem();
			}
		}
	}

	// 아이템 인스턴스가 없거나 수량이 0이면 절대 실행 불가
	if (!ItemInstance || ItemInstance->Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_Place: 아이템이 없거나 수량이 부족합니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ConsumableData = Cast<UCMDataAsset_ConsumableData>(ItemInstance->ItemData);

	if (!ConsumableData || ConsumableData->ConsumableType != EConsumableType::Placeable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_Place: 유효하지 않은 데이터입니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 비용 및 쿨타임
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 설치 동작 시작 시 손에 메쉬 부착
	AttachMeshToHand(ConsumableData.Get());
	
	// 몽타주 재생 및 이벤트 대기
	UAnimMontage* MontageToPlay = ConsumableData->ThrowableData.ThrowMontage; 
	
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, MontageToPlay
		);
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->ReadyForActivation();

		// WaitGameplayEvent 사용
		UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, CMGameplayTags::Player_Ability_Item_Place
		);
		EventTask->EventReceived.AddDynamic(this, &ThisClass::OnPlaceEventReceived);
		EventTask->ReadyForActivation();
	}
	else
	{
		// 몽타주 없으면 즉시 설치
		SpawnPlaceableActor();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UCMPlayerAbility_Place::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 종료 시 메쉬 제거
	DetachMeshFromHand();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMPlayerAbility_Place::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_Place::OnPlaceEventReceived(FGameplayEventData Payload)
{
	SpawnPlaceableActor();
}

void UCMPlayerAbility_Place::SpawnPlaceableActor()
{
	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarCharacter || !ConsumableData || !ConsumableData->PlaceableData.PlaceableActorClass)
	{
		return;
	}

	// 위치 계산
	FVector Start = AvatarCharacter->GetActorLocation();
	FVector End = Start - FVector(0, 0, 200.f);
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarCharacter);

	bool bHit = UKismetSystemLibrary::LineTraceSingle(
		this, Start, End, UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true
	);

	FTransform SpawnTransform;
	if (bHit)
	{
		SpawnTransform.SetLocation(HitResult.Location);
		if (ConsumableData->PlaceableData.bSnapToGround)
		{
			SpawnTransform.SetRotation(FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).ToQuat());
		}
	}
	else
	{
		SpawnTransform.SetLocation(AvatarCharacter->GetActorLocation());
	}

	// 서버에서 액터 스폰 및 수량 감소
	if (GetOwningActorFromActorInfo()->HasAuthority())
	{
		// 데이터 에셋에서 이미 ACMZoneEffectActor 타입으로 제한되어 있으므로 바로 사용
		TSubclassOf<ACMZoneEffectActor> ActorClass = ConsumableData->PlaceableData.PlaceableActorClass;
		
		ACMZoneEffectActor* ZoneActor = GetWorld()->SpawnActorDeferred<ACMZoneEffectActor>(
			ActorClass,
			SpawnTransform,
			AvatarCharacter,
			AvatarCharacter,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		if (ZoneActor)
		{
			// 메쉬 설정
			if (ConsumableData->PlaceableData.PlaceableMesh)
			{
				ZoneActor->SetMesh(ConsumableData->PlaceableData.PlaceableMesh);
			}

			// 데이터 전달
			ZoneActor->InitializeDuration(ConsumableData->PlaceableData.Duration);
			if (ConsumableData->ConsumableEffect)
			{
				ZoneActor->SetZoneGameplayEffect(ConsumableData->ConsumableEffect);
			}

			// 스폰 완료
			UGameplayStatics::FinishSpawningActor(ZoneActor, SpawnTransform);
			
			FindAndConsumeItem(ConsumableData, 1);
		}
	}
}
