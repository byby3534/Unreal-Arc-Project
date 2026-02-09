#include "UI/HUD/CMPlayerStatusWidget.h"
#include "Components/UI/UPawnUIComponent.h"
#include "Components/ProgressBar.h"

void UCMPlayerStatusWidget::InitPlayerStatus(UPawnUIComponent* PawnUIComponent)
{
	if (IsValid(PawnUIComponent))
	{
		CachedPawnUIComponent = PawnUIComponent;

		CachedPawnUIComponent->OnCurrentHealthChanged.AddDynamic(this, &ThisClass::OnHealthChanged);
		CachedPawnUIComponent->OnCurrentManaChanged.AddDynamic(this, &ThisClass::OnManaChanged);
		CachedPawnUIComponent->OnCurrentStaminaChanged.AddDynamic(this, &ThisClass::OnStaminaChanged);
		
		CachedPawnUIComponent->BroadcastInitStatusValues();
	}
}

void UCMPlayerStatusWidget::NativeDestruct()
{
	if (CachedPawnUIComponent.IsValid())
	{
		CachedPawnUIComponent->OnCurrentHealthChanged.RemoveDynamic(this, &ThisClass::OnHealthChanged);
		CachedPawnUIComponent->OnCurrentManaChanged.RemoveDynamic(this, &ThisClass::OnManaChanged);
		CachedPawnUIComponent->OnCurrentStaminaChanged.RemoveDynamic(this, &ThisClass::OnStaminaChanged);
	}

	Super::NativeDestruct();
}

void UCMPlayerStatusWidget::OnHealthChanged(float NewPercent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(NewPercent);
	}
}

void UCMPlayerStatusWidget::OnManaChanged(float NewPercent)
{
	if (ManaBar)
	{
		ManaBar->SetPercent(NewPercent);
	}
}

void UCMPlayerStatusWidget::OnStaminaChanged(float NewPercent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(NewPercent);
	}
}
