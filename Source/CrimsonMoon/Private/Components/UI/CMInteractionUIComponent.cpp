#include "Components/UI/CMInteractionUIComponent.h"
#include "UI/HUD/Interaction/CMInteractionWidget.h"
#include "Interfaces/CMInteractableInterface.h"

UCMInteractionUIComponent::UCMInteractionUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	bWantsInitializeComponent = true;
}

void UCMInteractionUIComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwningPlayerController = Cast<APlayerController>(GetOwner());
}

void UCMInteractionUIComponent::OnInteractableActorFound(AActor* NewTarget)
{
	if (CurrentInteractableActor.Get() != NewTarget)
	{
		if (CurrentInteractableActor.IsValid())
		{
			CurrentInteractableActor->OnDestroyed.RemoveDynamic(this, &UCMInteractionUIComponent::OnTargetActorDestroyed);
		}

		CurrentInteractableActor = NewTarget;

		if (NewTarget)
		{
			NewTarget->OnDestroyed.AddDynamic(this, &UCMInteractionUIComponent::OnTargetActorDestroyed);
		}
	}

	if (NewTarget)
	{
		if (NewTarget->Implements<UCMInteractableInterface>())
		{
			FInteractionUIData Data = ICMInteractableInterface::Execute_GetInteractableData(NewTarget);
			ShowInteractionUI(Data);	
		}
	}
	else
	{
		HideInteractionUI();
	}
}

void UCMInteractionUIComponent::ShowInteractionUI(const FInteractionUIData& Data)
{
	if (!IsValid(OwningPlayerController) || !OwningPlayerController->IsLocalController())
	{
		return;
	}

	// 상호작용 위젯 인스턴스가 없다면 생성
	if (!IsValid(InteractionWidgetInstance) && IsValid(InteractionWidgetClass))
	{
		InteractionWidgetInstance = CreateWidget<UCMInteractionWidget>(OwningPlayerController, InteractionWidgetClass);

		if (InteractionWidgetInstance)
		{
			InteractionWidgetInstance->AddToViewport(0);
		}
	}

	// 상호작용 위젯 데이터 업데이트 및 표시
	if (IsValid(InteractionWidgetInstance))
	{
		InteractionWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
		InteractionWidgetInstance->UpdateInteractionData(Data);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Missing InteractionWidgetClass!"), *GetName());
	}
}

void UCMInteractionUIComponent::UpdateInteractionProgress(float CurrentProgress)
{
	if (IsValid(InteractionWidgetInstance) && InteractionWidgetInstance->IsVisible())
	{
		InteractionWidgetInstance->UpdateProgressBar(CurrentProgress);
	}
}

void UCMInteractionUIComponent::HideInteractionUI()
{
	if (IsValid(InteractionWidgetInstance))
	{
		InteractionWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCMInteractionUIComponent::OnTargetActorDestroyed(AActor* DestroyedActor)
{
	HideInteractionUI();

	CurrentInteractableActor = nullptr;
}