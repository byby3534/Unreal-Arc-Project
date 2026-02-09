// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/CMEnemyCharacterBase.h"

#include "AIController.h"
#include "CMFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "GameplayTags/CMGameplayTags_Shared.h"
#include "GameplayTags/CMGameplayTags_Currency.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "UI/HUD/Enemy/CMEnemyHealthWidget.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/PlayerState.h"


ACMEnemyCharacterBase::ACMEnemyCharacterBase()
{
	bReplicates = true;

	EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>("Enemy Combat Component");

	#pragma region Collision Box
	RightHandCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Hand Collision Box"));
	RightHandCollisionBox->SetupAttachment(GetMesh(), RightHandCollisionBoxAttachBoneName);
	RightHandCollisionBox->SetBoxExtent(FVector(10.f, 10.f, 10.f));
	RightHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHandCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightHandCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RightHandCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnHandCollisionBeginOverlap);

	LeftHandCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Hand Collision Box"));
	LeftHandCollisionBox->SetupAttachment(GetMesh(), LeftHandCollisionBoxAttachBoneName);
	LeftHandCollisionBox->SetBoxExtent(FVector(10.f, 10.f, 10.f));
	LeftHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftHandCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftHandCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	LeftHandCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnHandCollisionBeginOverlap);
	#pragma endregion

	#pragma region Widget
	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComponent"));
	HealthBarWidgetComponent->SetupAttachment(GetRootComponent());

	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HealthBarWidgetComponent->SetDrawAtDesiredSize(true);
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	HealthBarWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
	#pragma endregion
}

void ACMEnemyCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACMEnemyCharacterBase, AIState);
	DOREPLIFETIME(ACMEnemyCharacterBase, StateTreeTargetActor);
}

#if WITH_EDITOR
void ACMEnemyCharacterBase::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// 오른손 콜리전 박스 부착 본 이름이 변경된 경우
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, RightHandCollisionBoxAttachBoneName))
	{
		if (RightHandCollisionBox && GetMesh())
		{
			RightHandCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightHandCollisionBoxAttachBoneName);
		}
	}

	// 왼손 콜리전 박스 부착 본 이름이 변경된 경우
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, LeftHandCollisionBoxAttachBoneName))
	{
		if (LeftHandCollisionBox && GetMesh())
		{
			LeftHandCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftHandCollisionBoxAttachBoneName);
		}
	}
}
#endif

void ACMEnemyCharacterBase::SetAIState(ECMEnemyState NewState)
{
	if (!HasAuthority())
	{
		return;
	}

	if (AIState == NewState)
	{
		return;
	}

	AIState = NewState;

	OnRep_AIState();
}

UEnemyCombatComponent* ACMEnemyCharacterBase::GetPawnCombatComponent() const
{
	return EnemyCombatComponent;
}

void ACMEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (PawnUIComponent)
	{
		PawnUIComponent->InitializeWithASC(GetAbilitySystemComponent());
	}

	if (HealthBarWidgetComponent)
	{
		UUserWidget* UserWidget = HealthBarWidgetComponent->GetUserWidgetObject();

		if (UCMEnemyHealthWidget* EnemyHealthWidget = Cast<UCMEnemyHealthWidget>(UserWidget))
		{
			EnemyHealthWidget->BindToEnemy(this);
		}
	}
}

void ACMEnemyCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// TeamID는 Controller 생성자에서 설정됨
	// Dead 태그 변경 감지 등록
	if (UCMAbilitySystemComponent* ASC = GetCMAbilitySystemComponent())
	{
		ASC->RegisterGameplayTagEvent(
			CMGameplayTags::Shared_Status_Dead,
			EGameplayTagEventType::NewOrRemoved
			).AddUObject(this, &ThisClass::OnStateTagChanged);

		// 서버에서만 Death 이벤트 수신 등록 (보상 지급용)
		if (HasAuthority())
		{
			DeathEventDelegateHandle = ASC->AddGameplayEventTagContainerDelegate(
				FGameplayTagContainer(CMGameplayTags::Shared_Event_Death),
				FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnDeathEventReceived)
				);
		}
	}
}

void ACMEnemyCharacterBase::OnRep_AIState()
{
	if (OnEnemyStateChanged.IsBound())
	{
		OnEnemyStateChanged.Broadcast(AIState);
	}

	/* TODO: AI 상태가 변경될 때 클라이언트에서 수행할 시각적 처리
	예: 상태에 따라 머티리얼 파라미터를 바꾸거나 이펙트를 재생
	*/
}

void ACMEnemyCharacterBase::OnStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (Tag == CMGameplayTags::Shared_Status_Dead && NewCount > 0)
	{
		// 상태를 Dead로 변경 -> UI 방송 자동 호출
		UE_LOG(LogTemp, Log, TEXT("[%s] 사망 태그 감지 -> AIState를 Dead로 변경"), *GetName());
		SetAIState(ECMEnemyState::Dead);
	}
	// (GAS와 연동될 영역)
	// 스테이트 트리는 이벤트를 기다리는 대신, 매 틱마다 "Stunned 태그가 있는가?"를
	// 스스로 검사하여 'Stunned' 상태로 즉시 전환(Global Transition)합니다.
}

void ACMEnemyCharacterBase::OnHandCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
	)
{
	if (!EnemyCombatComponent)
	{
		return;
	}

	APawn* HitPawn = Cast<APawn>(OtherActor);
	if (!HitPawn)
	{
		return;
	}

	// 자기 자신은 무시
	if (HitPawn == this)
	{
		return;
	}

	// 적대적인 대상만 공격
	if (UCMFunctionLibrary::IsTargetPawnHostile(this, HitPawn))
	{
		EnemyCombatComponent->OnHitTargetActor(OtherActor);
	}
}

FText ACMEnemyCharacterBase::GetEnemyName() const
{
	// 이름이 비어있을 경우를 대비한 방어 코드 (선택사항)
	if (EnemyName.IsEmpty())
	{
		// 이름 안 적었으면 "Unknown Enemy"라고라도 띄움
		return FText::FromString(TEXT("Unknown Enemy"));
	}

	return EnemyName;
}

void ACMEnemyCharacterBase::GrantKillReward(AActor* Killer)
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] GrantKillReward가 서버가 아닌 곳에서 호출됨: %s"), *GetName());
		return;
	}

	// 중복 지급 방지
	if (bRewardGiven)
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] %s의 보상이 이미 지급됨"), *GetName());
		return;
	}

	// 보상 금액이 0 이하면 지급하지 않음
	if (RewardCurrency <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[재화] %s는 지급할 재화가 없음"), *GetName());
		bRewardGiven = true; // 중복 호출 방지를 위해 플래그 설정
		return;
	}

	// Killer가 없으면 지급 불가
	if (!Killer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] GrantKillReward: %s의 처치자가 없음"), *GetName());
		return;
	}

	// Killer로부터 ASC 획득
	UCMAbilitySystemComponent* KillerASC = nullptr;

	// 1. Killer가 직접 IAbilitySystemInterface를 구현하는 경우
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Killer))
	{
		KillerASC = Cast<UCMAbilitySystemComponent>(ASCInterface->GetAbilitySystemComponent());
	}

	// 2. Killer가 Pawn이고 Controller나 PlayerState에 ASC가 있는 경우 (확장성)
	if (!KillerASC)
	{
		if (const APawn* KillerPawn = Cast<APawn>(Killer))
		{
			// Controller 확인
			if (AController* KillerController = KillerPawn->GetController())
			{
				if (const IAbilitySystemInterface* ControllerASC = Cast<IAbilitySystemInterface>(KillerController))
				{
					KillerASC = Cast<UCMAbilitySystemComponent>(ControllerASC->GetAbilitySystemComponent());
				}
			}

			// PlayerState 확인 (Controller에 없을 경우)
			if (!KillerASC)
			{
				if (APlayerState* PS = KillerPawn->GetPlayerState())
				{
					if (const IAbilitySystemInterface* PSASC = Cast<IAbilitySystemInterface>(PS))
					{
						KillerASC = Cast<UCMAbilitySystemComponent>(PSASC->GetAbilitySystemComponent());
					}
				}
			}
		}
	}

	if (!KillerASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] GrantKillReward: 처치자 %s의 ASC를 찾을 수 없음"), *Killer->GetName());
		return;
	}

	// 보상 지급 플래그 설정 (AddCurrency 호출 전에 설정하여 재진입 방지)
	bRewardGiven = true;

	// 재화 지급
	KillerASC->AddCurrency(RewardCurrency, CMGameplayTags::Currency_Reason_Kill, this);

	UE_LOG(LogTemp, Log, TEXT("[재화] %s 처치 -> %s에게 %d 재화 지급"), *GetName(), *Killer->GetName(), RewardCurrency);
}

void ACMEnemyCharacterBase::OnDeathEventReceived(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	// Payload가 없으면 리턴
	if (!Payload)
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] OnDeathEventReceived: %s의 Payload가 없음"), *GetName());
		return;
	}

	// Instigator (킬러)에게 보상 지급

	if (AActor* Killer = const_cast<AActor*>(Payload->Instigator.Get()))
	{
		GrantKillReward(Killer);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[재화] OnDeathEventReceived: %s의 처치자(Instigator)가 없음"), *GetName());
	}
}