// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Player/Common/CMPlayerAbility_EquipConsumable.h"
#include "DataAssets/Consumable/CMDataAsset_ConsumableData.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CMGameplayTags.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "Items/Object/CMItemInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCMPlayerAbility_EquipConsumable::UCMPlayerAbility_EquipConsumable()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer TagsToAdd;
	TagsToAdd.AddTag(CMGameplayTags::Player_Ability_EquipConsumable);
	SetAssetTags(TagsToAdd);

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = CMGameplayTags::Player_Event_EquipConsume;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UCMPlayerAbility_EquipConsumable::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 현재 연관된 아이템 인스턴스 가져오기
	UCMItemInstance* ItemInstance = GetAssociatedItemInstance();

	// 트리거 이벤트 데이터에서 아이템 인스턴스를 가져올 수 있는지 확인
	if (!ItemInstance && TriggerEventData && TriggerEventData->OptionalObject)
	{
		const UObject* RawObject = TriggerEventData->OptionalObject.Get();
		
		// OptionalObject가 ItemInstance일 수도 있고 DataAsset일 수도 있음
		if (UCMItemInstance* TriggerItem = Cast<UCMItemInstance>(const_cast<UObject*>(RawObject)))
		{
			ItemInstance = TriggerItem;
		}
	}

	// 그래도 없으면 퀵바에서 활성 아이템 가져오기
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

	// 아이템이 없거나 수량이 0이면 실행 불가
	if (!ItemInstance || ItemInstance->Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_EquipConsumable: 아이템이 없거나 수량이 부족합니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 데이터 캐싱
	ConsumableData = Cast<const UCMDataAsset_ConsumableData>(ItemInstance->ItemData);

	if (!ConsumableData)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipConsumable: 데이터가 없습니다. 종료합니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 타입에 따른 분기 처리
	if (ConsumableData->ConsumableType == EConsumableType::Immediate)
	{
		// [즉발형] -> 바로 사용 로직 진입
		HandleImmediateItem();
	}
	else
	{
		// [장착형] -> 손에 들고 대기
		HandleEquippableItem(ItemInstance);
	}
}

void UCMPlayerAbility_EquipConsumable::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ConsumableData && ConsumableData->ConsumableType == EConsumableType::Immediate)
	{
		DetachMeshFromHand();
	}

	// 부여했던 하위 어빌리티(투척/설치 등) 회수
	UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();
	if (ASC && !GrantedAbilityHandles.IsEmpty())
	{
		// 배열을 복사하여 순회 중 원본 수정 크래시 방지
		TArray<FGameplayAbilitySpecHandle> HandlesToRemove = GrantedAbilityHandles;
		GrantedAbilityHandles.Empty();

		for (const FGameplayAbilitySpecHandle& SpecHandle : HandlesToRemove)
		{
			if (SpecHandle.IsValid())
			{
				ASC->ClearAbility(SpecHandle);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCMPlayerAbility_EquipConsumable::OnEquipEventReceived(FGameplayEventData Payload)
{
	AttachMeshToHand(ConsumableData.Get());
}

void UCMPlayerAbility_EquipConsumable::OnConsumeEventReceived(FGameplayEventData Payload)
{
	// 실제 효과 적용 및 개수 차감은 서버에서만 수행
	if (GetOwningActorFromActorInfo()->HasAuthority())
	{
		// GE 적용
		if (ConsumableData->ConsumableEffect)
		{
			UAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			ASC->ApplyGameplayEffectToSelf(
				ConsumableData->ConsumableEffect.GetDefaultObject(),
				1.0f,
				ContextHandle
			);
		}
		else
		{
			// [디버그 에러] 데이터 없음
			UE_LOG(LogTemp, Error, TEXT("Error: ConsumableData가 없거나 Effect 클래스가 비어있습니다!"));
		}

		FindAndConsumeItem(ConsumableData, 1);
	}
}

// 즉발형 아이템 처리
void UCMPlayerAbility_EquipConsumable::HandleImmediateItem()
{
	AttachMeshToHand(ConsumableData.Get());

	// 마시는 몽타주 재생
	UAnimMontage* MontageToPlay = ConsumableData->ImmediateData.UseMontage;
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, MontageToPlay, 1.0f, NAME_None, false
		);

		// 몽타주가 끝나거나 중단되면 어빌리티도 종료
		MontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
		MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
		MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);
		MontageTask->ReadyForActivation();

		// 효과 적용 타이밍 대기 (노티파이: Foley.Event.Immediate)
		UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, CMGameplayTags::Foley_Event_Immediate
		);
		EventTask->EventReceived.AddDynamic(this, &ThisClass::OnConsumeEventReceived);
		EventTask->ReadyForActivation();
	}
	else
	{
		// 몽타주가 없으면 즉시 적용 후 종료
		FGameplayEventData DummyPayload;
		OnConsumeEventReceived(DummyPayload);
		K2_EndAbility(); 
	}
}

// 장착형 아이템 처리
void UCMPlayerAbility_EquipConsumable::HandleEquippableItem(UCMItemInstance* InItemInstance)
{
	UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponentFromActorInfo();
    
	// 실행할 어빌리티 클래스가 있는지 확인
	if (ASC && ConsumableData->ActivationAbility)
	{
		// 서버 권한이 있을 때만 실행 (클라이언트에서 실행 시 핸들 불일치 문제 발생)
		if (GetOwningActorFromActorInfo()->HasAuthority())
		{
			UObject* SourceObj = InItemInstance ? (UObject*)InItemInstance : (UObject*)ConsumableData;

			// 어빌리티를 부여하고 즉시 한 번 실행
			FGameplayAbilitySpec AbilitySpec(ConsumableData->ActivationAbility, 1, -1, SourceObj);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(CMGameplayTags::Player_Ability_Item);
	        
			// 투척/설치 어빌리티를 즉시 실행
			ASC->GiveAbilityAndActivateOnce(AbilitySpec);
		}
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}