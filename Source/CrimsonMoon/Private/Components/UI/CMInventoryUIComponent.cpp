#include "Components/UI/CMInventoryUIComponent.h"
#include "Components/UI/UIManagerComponent.h"
#include "Components/CMInventoryComponent.h"
#include "UI/Inventory/CMInventoryWidget.h"

UCMInventoryUIComponent::UCMInventoryUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UCMInventoryUIComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		UIManager = PC->GetComponentByClass<UUIManagerComponent>();
	}
}

void UCMInventoryUIComponent::InitializeInventoryUI(UCMInventoryComponent* InInventoryComponent, UCMQuickBarComponent* InQuickBarComponent)
{
	CachedInventoryComponent = InInventoryComponent;
	CachedQuickBarComponent = InQuickBarComponent;
}

void UCMInventoryUIComponent::OpenInventoryUI()
{
	if (!UIManager || !InventoryWidgetClass)
	{
		return;
	}

	UUserWidget* Widget = UIManager->PushWidget(InventoryWidgetClass);

	if (UCMInventoryWidget* InventoryWidget = Cast<UCMInventoryWidget>(Widget))
	{
		InventoryWidget->InitInventoryWidget(CachedInventoryComponent, CachedQuickBarComponent);
	}
}