// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_Throw.h"
#include "DataAssets/Consumable/CMDataAsset_ConsumableData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CMGameplayTags.h"
#include "Items/Object/CMItemInstance.h"
#include "Items/Actors/CMThrowProjectileActor.h"
#include "DrawDebugHelpers.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "Kismet/GameplayStatics.h"

UCMPlayerAbility_Throw::UCMPlayerAbility_Throw()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_Item_Throw);
	
	ThrowEventTag = CMGameplayTags::Foley_Event_Throw;
}

void UCMPlayerAbility_Throw::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UCMItemInstance* ItemInstance = GetAssociatedItemInstance();
	
	// 아이템 인스턴스가 없으면 퀵바에서 찾기 시도
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

	//  아이템 인스턴스가 없거나 수량이 0 이하면 무조건 실패 처리
	if (!ItemInstance || ItemInstance->Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_Throw: 아이템이 없거나 수량이 부족하여 취소"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//  데이터 에셋 캐싱
	ConsumableData = Cast<const UCMDataAsset_ConsumableData>(ItemInstance->ItemData);

	if (!ConsumableData || ConsumableData->ConsumableType != EConsumableType::Throwable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_Throw: 유효하지 않는 데이터 에셋이거나, 투척 타입이 아님."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 비용 및 쿨타임 확인
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 투척 시작 시 손에 메쉬 부착
	AttachMeshToHand(ConsumableData.Get());
	
	// 몽타주 재생 및 이벤트 대기
	UAnimMontage* MontageToPlay = ConsumableData->ThrowableData.ThrowMontage;
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, MontageToPlay, 1.0f, NAME_None, false
		);

		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
		MontageTask->ReadyForActivation();

		// WaitGameplayEvent 사용
		UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, ThrowEventTag
		);
		EventTask->EventReceived.AddDynamic(this, &ThisClass::OnThrowEventReceived);
		EventTask->ReadyForActivation();
	}
	else
	{
		// 몽타주 없으면 즉시 발사
		SpawnProjectile();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UCMPlayerAbility_Throw::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 어빌리티 종료 시 손에서 메쉬 제거
	DetachMeshFromHand();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMPlayerAbility_Throw::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCMPlayerAbility_Throw::OnThrowEventReceived(FGameplayEventData Payload)
{
	SpawnProjectile();
}

void UCMPlayerAbility_Throw::SpawnProjectile()
{
	// 서버와 클라이언트 모두에서 실행
	if (!ConsumableData || !ConsumableData->ThrowableData.ProjectileClass)
	{
		return;
	}

	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarCharacter)
	{
		return;
	}
	
	FVector SpawnLocation;
	FRotator SpawnRotation = AvatarCharacter->GetController()->GetControlRotation();

	FName SocketName = ConsumableData->ThrowableData.ThrowSocketName;

	if (!SocketName.IsNone() && AvatarCharacter->GetMesh()->DoesSocketExist(SocketName))
	{
		SpawnLocation = AvatarCharacter->GetMesh()->GetSocketLocation(SocketName);
	}
	else
	{
		SpawnLocation = AvatarCharacter->GetActorLocation() + (AvatarCharacter->GetActorForwardVector() * 100.f);
		UE_LOG(LogTemp, Warning, TEXT("데이터 에셋에서 소켓을 설정하지 않았거나 찾을 수 없습니다. 캐릭터 전방에서 발사합니다."));
	}

	FTransform SpawnTransform(SpawnRotation, SpawnLocation);



	// 서버에서만 실제 발사체 생성
	if (GetOwningActorFromActorInfo()->HasAuthority())
	{
		// 액터 스폰
		AActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(
		  ConsumableData->ThrowableData.ProjectileClass, 
		  SpawnTransform, 
		  GetOwningActorFromActorInfo(),
		  AvatarCharacter,
		  ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	   );

		ACMThrowProjectileActor* ProjectileActor = Cast<ACMThrowProjectileActor>(SpawnedActor);

		if (ProjectileActor)
		{
			// 이펙트 스펙 생성 및 전달
			if (ConsumableData->ConsumableEffect)
			{
				FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ConsumableData->ConsumableEffect);
				ProjectileActor->ProjectileEffectSpec = SpecHandle;
			}

			// 발사체 초기화
			ProjectileActor->InitializeProjectile(ConsumableData->ThrowableData);
			if (ConsumableData->HandHeldMesh)
			{
				ProjectileActor->SetProjectileMesh(ConsumableData->HandHeldMesh);
			}
			
			UGameplayStatics::FinishSpawningActor(ProjectileActor, SpawnTransform);
			
			if (UProjectileMovementComponent* PMC = ProjectileActor->FindComponentByClass<UProjectileMovementComponent>())
			{
				PMC->Velocity = SpawnRotation.Vector() * ConsumableData->ThrowableData.ThrowSpeed;
			}
			
			FindAndConsumeItem(ConsumableData, 1);
		}
	}
}