// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Input/CMPickUpComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Interfaces/CMInteractableInterface.h"
#include "Net/UnrealNetwork.h"

UCMPickUpComponent::UCMPickUpComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
	SetIsReplicatedByDefault(true);
}

void UCMPickUpComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 현재 상호작용 가능한 액터를 복제 (서버 -> 클라이언트)
	DOREPLIFETIME(UCMPickUpComponent, CurrentInteractableActor);
}

void UCMPickUpComponent::BeginPlay()
{
	Super::BeginPlay();
	
	bNeedUpdate = true;

	// 컴포넌트가 시작되자마자 초기화 시도
	InitializeInputSystem();

	// [오버랩 이벤트 바인딩]
	if (GetOwner()->HasAuthority())
	{
		if (AActor* Owner = GetOwner())
		{
			Owner->OnActorBeginOverlap.AddDynamic(this, &UCMPickUpComponent::OnOwnerOverlapBegin);
			Owner->OnActorEndOverlap.AddDynamic(this, &UCMPickUpComponent::OnOwnerOverlapEnd);
		}
	}
}

void UCMPickUpComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RetryInitTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UCMPickUpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 서버에서만 가장 가까운 아이템을 계산
	if (GetOwner()->HasAuthority())
	{
		// 업데이트가 필요할 때만 거리 계산 수행
		if (bNeedUpdate || OverlappingActors.Num() > 1)
		{
			FindClosestInteractable();
			bNeedUpdate = false;
		}
	}
}

void UCMPickUpComponent::InitializeInputSystem()
{
	
	// 주인(Owner) 확인
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// ASC 찾기
	if (!AbilitySystemComponent.IsValid())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
		{
			AbilitySystemComponent = ASI->GetAbilitySystemComponent();
		}
		else if (UAbilitySystemComponent* FoundASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
		{
			AbilitySystemComponent = FoundASC;
		}
	}

	// 폰이 아니면 입력 바인딩 불가
	APawn* OwnerPawn = Cast<APawn>(Owner);
	if (!OwnerPawn)
	{
		return;
	}

	// InputComponent 바인딩 (액션 바인딩)
	if (UInputComponent* OwnerInputComp = OwnerPawn->InputComponent)
	{
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(OwnerInputComp))
		{
			if (PickupAction)
			{
				EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Started, this, &UCMPickUpComponent::OnPickupInput);
			}
		}
	}

	// Subsystem을 통한 매핑 컨텍스트 추가 (가장 중요: 실패 가능성 높음)
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	
	// 컨트롤러가 없거나 로컬 플레이어가 아니면 아직 준비가 안 된 것
	if (!PC || !PC->GetLocalPlayer())
	{
		// 아직 컨트롤러가 빙의되지 않았으므로 잠시 후 재시도
		if (UWorld* World = GetWorld())
		{
			if (!World->GetTimerManager().IsTimerActive(RetryInitTimerHandle))
			{
				// 0.2초 뒤에 다시 시도
				World->GetTimerManager().SetTimer(RetryInitTimerHandle, this, &UCMPickUpComponent::InitializeInputSystem, 0.2f, false);
			}
		}
		return;
	}

	// 컨트롤러가 있다면 서브시스템 접근 시도
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (PickupMappingContext)
		{
			// 매핑 컨텍스트 추가
			Subsystem->AddMappingContext(PickupMappingContext, 0);
		}
		
		// 성공했으므로 재시도 타이머가 있다면 제거
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RetryInitTimerHandle);
		}
	}
	else
	{
		// 컨트롤러는 있는데 서브시스템을 못 가져오는 경우 재시도
		if (UWorld* World = GetWorld())
		{
			if (!World->GetTimerManager().IsTimerActive(RetryInitTimerHandle))
			{
				World->GetTimerManager().SetTimer(RetryInitTimerHandle, this, &UCMPickUpComponent::InitializeInputSystem, 0.2f, false);
			}
		}
	}
}

void UCMPickUpComponent::OnPickupInput(const FInputActionValue& Value)
{
	// 여기서 직접 트레이스를 하지 않고, 어빌리티 시스템에 신호를 보냄
	if (AbilitySystemComponent.IsValid() && InteractAbilityTag.IsValid())
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(InteractAbilityTag.GetSingleTagContainer(), true);
	}
}

void UCMPickUpComponent::OnOwnerOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	// 상호작용 가능한 물체만 리스트에 추가
	if (OtherActor && OtherActor->Implements<UCMInteractableInterface>())
	{
		OverlappingActors.AddUnique(OtherActor);
		bNeedUpdate = true;
	}
}

void UCMPickUpComponent::OnOwnerOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OtherActor)
	{
		OverlappingActors.Remove(OtherActor);
		bNeedUpdate = true;
	}
}

void UCMPickUpComponent::FindClosestInteractable()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	AActor* ClosestActor = nullptr;
	float MinDistanceSq = FLT_MAX;
	FVector OwnerLocation = Owner->GetActorLocation();

	// 리스트를 역순으로 순회
	for (int32 i = OverlappingActors.Num() - 1; i >= 0; --i)
	{
		AActor* Target = OverlappingActors[i].Get();

		// 유효하지 않거나 파괴된 액터 정리
		if (!Target || !IsValid(Target))
		{
			OverlappingActors.RemoveAt(i);
			continue;
		}

		// 거리 제곱 계산
		float DistSq = FVector::DistSquared(OwnerLocation, Target->GetActorLocation());

		if (DistSq < MinDistanceSq)
		{
			MinDistanceSq = DistSq;
			ClosestActor = Target;
		}
	}

	// 타겟이 바뀌었을 때만 델리게이트 방송
	if (ClosestActor != CurrentInteractableActor.Get())
	{
		CurrentInteractableActor = ClosestActor;
		OnInteractableFound.Broadcast(ClosestActor);
	}
}

void UCMPickUpComponent::OnRep_CurrentInteractableActor()
{
	// 클라이언트에서 값이 변경되면 UI 갱신
	OnInteractableFound.Broadcast(CurrentInteractableActor.Get());
}
