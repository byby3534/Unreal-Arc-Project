#include "Controllers/CMPlayerControllerBase.h"
#include "Components/UI/UIManagerComponent.h"
#include "Components/Input/CMInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "CrimsonMoon/DebugHelper.h"
#include "DataAssets/Input/CMDataAsset_InputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "UI/Core/CMBaseWidget.h"
#include "CMGameplayTags.h"

ACMPlayerControllerBase::ACMPlayerControllerBase()
{
	UIManagerComponent = CreateDefaultSubobject<UUIManagerComponent>(TEXT("UIManagerComponent"));
}

void ACMPlayerControllerBase::Init()
{
	// 로컬 컨트롤러에서만 처리
	if (!IsLocalController())
	{
		return;
	}

	if (StartWidgetClass && UIManagerComponent)
	{
		UIManagerComponent->ResetUI();
		UIManagerComponent->PushWidget(StartWidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] StartWidgetClass or UIManagerComponent is invalid in OnPossess."), *GetName());
	}

	if (!InputConfigUIDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] InputConfigDataAsset is invalid."), *GetName());
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputConfigUIDataAsset->DefaultMappingContext)
		{
			Subsystem->AddMappingContext(InputConfigUIDataAsset->DefaultMappingContext, 0);
		}
	}
}

void ACMPlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ACMPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	// BeginPlay에서는 더 이상 UI 초기화를 하지 않음. Pawn이 실제로 할당된 이후(OnRep_Pawn) 시점에 처리.
}

void ACMPlayerControllerBase::OnRep_Pawn()
{
	Super::OnRep_Pawn();

}

void ACMPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	UCMInputComponent* CMInput = Cast<UCMInputComponent>(InputComponent);
	if (!CMInput)
	{
		return;	
	}

	if (InputConfigUIDataAsset)
	{
		CMInput->BindNativeInputAction(InputConfigUIDataAsset, CMGameplayTags::InputTag_UI_Back, ETriggerEvent::Started, this, &ThisClass::HandleBackAction);
	}
}

void ACMPlayerControllerBase::HandleBackAction()
{
	if (UIManagerComponent && UIManagerComponent->GetTopWidget())
	{
		UIManagerComponent->OnBackAction();
	}
	else
	{
		OnSystemMenu();
	}
}

void ACMPlayerControllerBase::OnSystemMenu()
{
}
