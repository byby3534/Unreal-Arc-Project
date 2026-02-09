// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/CMPlayerCharacterBase.h"
#include "CMFunctionLibrary.h"
#include "CMGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CMLineTraceComponent.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Components/Input/CMInputComponent.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "Controllers/CMPlayerControllerBase.h"
#include "DataAssets/Input/CMDataAsset_InputConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "DataAssets/Weapon/CMDataAsset_WeaponData.h"

ACMPlayerCharacterBase::ACMPlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본 회전 세팅
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PlayerCombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("Player Combat Component"));
	LineTraceComponent = CreateDefaultSubobject<UCMLineTraceComponent>(TEXT("LineTraceComponent"));

	#pragma region Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 120.f);
	CameraBoom->bUsePawnControlRotation = true;

	CameraBoom->bAutoActivate = false;
	#pragma endregion

	#pragma region View Camera
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("View Camera"));
	ViewCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ViewCamera->bAutoActivate = false;
	#pragma endregion

	#pragma region Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	#pragma endregion

	#pragma region HandleMesh
	HandHeldItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandHeldItemMesh"));
	HandHeldItemMesh->SetupAttachment(GetMesh(), FName("Hand_R_Socket"));
	HandHeldItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HandHeldItemMesh->SetCastShadow(true);
	#pragma endregion
}

void ACMPlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACMPlayerCharacterBase, CachedRollInputDirection);
}

void ACMPlayerCharacterBase::CacheRollInputDirection()
{
	// 현재 Movement Input 방향 저장
	CachedRollInputDirection = GetLastMovementInputVector();
}

void ACMPlayerCharacterBase::ServerCacheRollInputDirection_Implementation(FVector_NetQuantize100 InputDirection)
{
	CachedRollInputDirection = InputDirection;
}

void ACMPlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterMovement())
	{
		CachedDefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}

	// Grant Character Class Tags to ASC (플레이어 캐릭터만)
	if (CMAbilitySystemComponent)
	{
		// 모든 플레이어에게 Common 태그 부여 (공용 아이템/무기 사용 가능)
		CMAbilitySystemComponent->AddLooseGameplayTag(CMGameplayTags::Character_Class_Common);

		// 플레이어별 클래스 태그 부여 (Blade, Arcanist 등)
		if (CharacterClassTag.IsValid())
		{
			CMAbilitySystemComponent->AddLooseGameplayTag(CharacterClassTag);
		}
	}

	// BeginPlay 에서는 카메라 셋업을 하지 않음
}

void ACMPlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//(서버 전용, 자동으로 클라이언트에 복제됨)
	if (CMAbilitySystemComponent)
	{
		// 모든 플레이어에게 Common 태그 부여 (공용 아이템/무기 사용 가능)
		CMAbilitySystemComponent->AddLooseGameplayTag(CMGameplayTags::Character_Class_Common);

		// 플레이어별 클래스 태그 부여 (Blade, Arcanist 등)
		if (CharacterClassTag.IsValid())
		{
			CMAbilitySystemComponent->AddLooseGameplayTag(CharacterClassTag);
		}
	}

	// 카메라 셋업은 PawnClientRestart 에서 로컬 컨트롤 기준으로 처리
}

void ACMPlayerCharacterBase::SetupGameplayCamera_Helper()
{
	UE_LOG(LogTemp, Warning, TEXT("SetupGameplayCamera_Helper: Called. IsLocallyControlled=%d"), IsLocallyControlled() ? 1 : 0);

	// 로컬 플레이어가 소유한 Pawn 에서만 카메라 셋업
	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupGameplayCamera_Helper: Aborted - not locally controlled"));
		return;
	}

	// BP 에서 추가된 UGameplayCameraComponent 를 찾아 SetupCamera 호출
	if (UGameplayCameraComponent* GameplayCameraComp = FindComponentByClass<UGameplayCameraComponent>())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupGameplayCamera_Helper: Found GameplayCameraComponent on %s, calling SetupCamera"), *GetName());
		GameplayCameraComp->Activate();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupGameplayCamera_Helper: GameplayCameraComponent not found on %s"), *GetName());
	}
}

void ACMPlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled() || !CMAbilitySystemComponent)
	{
		return;
	}

	if (CMAbilitySystemComponent->HasMatchingGameplayTag(CMGameplayTags::Shared_Status_IsLockedOn))
	{
		if (PlayerCombatComponent && PlayerCombatComponent->CurrentLockOnTarget.IsValid())
		{
			AActor* CurrentTarget = PlayerCombatComponent->CurrentLockOnTarget.Get();

			// 타겟 사망시 락온 자동 해제
			if (UCMFunctionLibrary::NativeDoesActorHaveTag(CurrentTarget, CMGameplayTags::Shared_Status_Dead))
			{
				CMAbilitySystemComponent->OnAbilityInputPressed(CMGameplayTags::InputTag_Toggleable_LockOn);
				return;
			}

			if (GetController())
			{
				const FVector CharacterLocation = GetActorLocation();
				const FVector TargetLocation = CurrentTarget->GetActorLocation();
				FRotator LookAtRotator = UKismetMathLibrary::FindLookAtRotation(CharacterLocation, TargetLocation);

				const FRotator CurrentControlRotator = GetControlRotation();
				const FRotator TargetRotator = FMath::RInterpTo(CurrentControlRotator, LookAtRotator, DeltaTime, TargetLockRotationInterpSpeed);

				GetController()->SetControlRotation(TargetRotator);
				SetActorRotation(FRotator(0.f, TargetRotator.Yaw, 0.f));
			}
		}
	}
}

void ACMPlayerCharacterBase::PawnClientRestart()
{
	Super::PawnClientRestart();


	if (ACMPlayerControllerBase* CMPC = Cast<ACMPlayerControllerBase>(GetController()))
	{
		CMPC->Init();
	}
	
	if (const APlayerController* OwningPlayerController = GetController<APlayerController>())
	{
		UEnhancedInputLocalPlayerSubsystem* PlayerSubsystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

		check(PlayerSubsystem);

		PlayerSubsystem->RemoveMappingContext(InputConfigDataAsset->DefaultMappingContext);
		PlayerSubsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);
	}
	
	// 로컬 클라이언트가 이 Pawn 을 다시 사용할 준비가 된 시점에 카메라 셋업 시도
	SetupGameplayCamera_Helper();
}

UPlayerCombatComponent* ACMPlayerCharacterBase::GetPawnCombatComponent() const
{
	return PlayerCombatComponent;
}

void ACMPlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UCMInputComponent* CMInputComponent = CastChecked<UCMInputComponent>(PlayerInputComponent);

	// [디버그 로그 추가] 데이터 에셋이 잘 들어왔는지 확인
	if (!InputConfigDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("[오류] InputConfigDataAsset이 비어있습니다! BP를 확인하세요."));
		return;
	}

	// [디버그 로그 추가] 태그로 액션을 찾을 수 있는지 확인
	const FGameplayTag PotionTag = CMGameplayTags::InputTag_QuickSlot_Potion;
	UInputAction* PotionAction = InputConfigDataAsset->FindNativeInputActionByTag(PotionTag);

	if (PotionAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("[성공] 포션 태그(%s)에 해당하는 IA(%s)를 찾았습니다. 바인딩 시도..."), 
			*PotionTag.ToString(), *PotionAction->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[실패] 포션 태그(%s)로 IA를 찾을 수 없습니다! DA 설정을 확인하세요."), *PotionTag.ToString());
	}

	// 입력키 바인딩 방법
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_Jump, ETriggerEvent::Triggered, this, &ThisClass::Jump);

	// 즉발형
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_QuickSlot_Potion, ETriggerEvent::Started, this, &ThisClass::Input_QuickSlot_Potion_Started);
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_QuickSlot_Potion, ETriggerEvent::Canceled, this, &ThisClass::Input_QuickSlot_Potion_Tap);
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_QuickSlot_Potion, ETriggerEvent::Triggered, this, &ThisClass::Input_QuickSlot_Potion_Hold);

	// 유틸리티형
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_QuickSlot_Utility, ETriggerEvent::Started, this, &ThisClass::Input_QuickSlot_Utility_Started);
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_QuickSlot_Utility, ETriggerEvent::Canceled, this, &ThisClass::Input_QuickSlot_Utility_Tap);
	CMInputComponent->BindNativeInputAction(InputConfigDataAsset, CMGameplayTags::InputTag_QuickSlot_Utility, ETriggerEvent::Triggered, this, &ThisClass::Input_QuickSlot_Utility_Hold);
	
	// GA 자동 바인딩
	CMInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, &ThisClass::Input_AbilityInputPressed, &ThisClass::Input_AbilityInputReleased);
}

void ACMPlayerCharacterBase::Input_AbilityInputPressed(const FGameplayTag InInputTag)
{
	// Roll 입력일 때 현재 Movement Input 방향 캐싱
	if (InInputTag.MatchesTagExact(CMGameplayTags::InputTag_Roll))
	{
		CacheRollInputDirection();

		// 클라이언트인 경우 서버에 즉시 전송
		if (!HasAuthority())
		{
			ServerCacheRollInputDirection(CachedRollInputDirection);
		}
	}

	if (CMAbilitySystemComponent)
	{
		CMAbilitySystemComponent->OnAbilityInputPressed(InInputTag);
	}
}

void ACMPlayerCharacterBase::Input_AbilityInputReleased(const FGameplayTag InInputTag)
{
	if (CMAbilitySystemComponent)
	{
		CMAbilitySystemComponent->OnAbilityInputReleased(InInputTag);
	}
}

void ACMPlayerCharacterBase::Input_Move(const FInputActionValue& InputActionValue)
{
	if (CMAbilitySystemComponent && CMAbilitySystemComponent->HasMatchingGameplayTag(CMGameplayTags::Player_Status_Charging))
	{
		return;
	}

	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);

	if (MovementVector.Y != 0.f)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);

		AddMovementInput(ForwardDirection, MovementVector.Y);
	}

	if (MovementVector.X != 0.f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACMPlayerCharacterBase::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if (CMAbilitySystemComponent && CMAbilitySystemComponent->HasMatchingGameplayTag(CMGameplayTags::Shared_Status_IsLockedOn))
	{

		if (PlayerCombatComponent && FMath::Abs(LookAxisVector.X) > 0.5f)
		{
			PlayerCombatComponent->Server_SwitchLockOnTarget(LookAxisVector);
		}
	}
	else
	{

		if (LookAxisVector.X != 0.f)
		{
			AddControllerYawInput(LookAxisVector.X);
		}
		if (LookAxisVector.Y != 0.f)
		{
			AddControllerPitchInput(LookAxisVector.Y);
		}
	}
}

void ACMPlayerCharacterBase::Input_QuickSlot_Potion_Started(const FInputActionValue& Value)
{
	// 누르는 순간 호출
	LastPotionSwapTime = 0.0;
}

void ACMPlayerCharacterBase::Input_QuickSlot_Potion_Tap(const FInputActionValue& InputActionValue)
{
	Server_SetActiveSlot(1);
	UE_LOG(LogTemp, Log, TEXT("[Input] Potion Tap: Use Item"));
}

void ACMPlayerCharacterBase::Input_QuickSlot_Potion_Hold(const FInputActionValue& InputActionValue)
{
	double CurrentTime = GetWorld()->GetTimeSeconds();

	// 마지막 스왑으로부터 시간이 지났는지 확인
	if (CurrentTime - LastPotionSwapTime >= QuickSlotSwapInterval)
	{
		UCMQuickBarComponent* QuickBar = FindComponentByClass<UCMQuickBarComponent>();
		if (QuickBar)
		{
			QuickBar->CycleSlotItem(1);
			UE_LOG(LogTemp, Log, TEXT("[Input] Potion Hold: Cycle (Time: %f)"), CurrentTime);
		}

		// 마지막 스왑 시간 갱신
		LastPotionSwapTime = CurrentTime;
	}
}

void ACMPlayerCharacterBase::Input_QuickSlot_Utility_Started(const FInputActionValue& Value)
{
	LastUtilitySwapTime = 0.0;
}

void ACMPlayerCharacterBase::Input_QuickSlot_Utility_Tap(const FInputActionValue& InputActionValue)
{
	Server_SetActiveSlot(2);
	UE_LOG(LogTemp, Log, TEXT("[Input] Utility Tap: Use Item"));
}

void ACMPlayerCharacterBase::Input_QuickSlot_Utility_Hold(const FInputActionValue& InputActionValue)
{
	double CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastUtilitySwapTime >= QuickSlotSwapInterval)
	{
		UCMQuickBarComponent* QuickBar = FindComponentByClass<UCMQuickBarComponent>();
		if (QuickBar)
		{
			QuickBar->CycleSlotItem(2);
			UE_LOG(LogTemp, Log, TEXT("[Input] Utility Hold: Cycle (Time: %f)"), CurrentTime);
		}
		LastUtilitySwapTime = CurrentTime;
	}
}

void ACMPlayerCharacterBase::Server_SetActiveSlot_Implementation(int32 SlotIndex)
{
	UCMQuickBarComponent* QuickBar = FindComponentByClass<UCMQuickBarComponent>();
	if (QuickBar)
	{
		QuickBar->SetActiveSlot(SlotIndex);
	}
}

void ACMPlayerCharacterBase::Server_CycleActiveItem_Implementation(int32 Direction)
{
	// 서버에서는 컴포넌트를 직접 찾아 로직을 수행합니다.
	if (UCMQuickBarComponent* QuickBar = FindComponentByClass<UCMQuickBarComponent>())
	{
		// 현재 활성화된 슬롯을 확인하고 순환
		int32 ActiveSlot = QuickBar->GetActiveSlotIndex();
		
		if (ActiveSlot == 0 || ActiveSlot == 2)
		{
			QuickBar->CycleSlotItem(ActiveSlot, Direction);
		}
	}
}

void ACMPlayerCharacterBase::SetGameplayInputEnabled(bool bEnabled)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (CurrentWeaponMappingContext)
			{
				Subsystem->RemoveMappingContext(CurrentWeaponMappingContext);
			}

			if (InputConfigDataAsset && InputConfigDataAsset->DefaultMappingContext)
			{
				Subsystem->RemoveMappingContext(InputConfigDataAsset->DefaultMappingContext);
			}

			UInputMappingContext* WeaponIMC = nullptr;
			if (PlayerCombatComponent)
			{
				if (const UCMDataAsset_WeaponData* WeaponData = PlayerCombatComponent->GetPlayerCurrentWeaponData())
				{
					WeaponIMC = WeaponData->WeaponInputMappingContext;
				}
			}

			CurrentWeaponMappingContext = WeaponIMC;

			if (bEnabled)
			{
				if (InputConfigDataAsset && InputConfigDataAsset->DefaultMappingContext)
				{
					Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);
				}

				if (CurrentWeaponMappingContext)
				{
					Subsystem->AddMappingContext(CurrentWeaponMappingContext, 1);
				}
			}
			else
			{
				if (InputConfigDataAsset && InputConfigDataAsset->DefaultMappingContext)
				{
					Subsystem->RemoveMappingContext(InputConfigDataAsset->DefaultMappingContext);
				}

				if (CurrentWeaponMappingContext)
				{
					Subsystem->RemoveMappingContext(CurrentWeaponMappingContext);
				}
			}
		}
	}
}
