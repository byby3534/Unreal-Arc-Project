#include "UI/HUD/Enemy/CMEnemyHealthWidget.h"
#include "Components/UI/UPawnUIComponent.h"
#include "Components/ProgressBar.h"
#include "TimerManager.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Kismet/GameplayStatics.h"

UCMEnemyHealthWidget::UCMEnemyHealthWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsLockedOn = false;
	bIsHealthChanged = false;
	ShowDuration = 3.0f;
}

void UCMEnemyHealthWidget::BindToEnemy(ACMEnemyCharacterBase* InEnemyCharacter)
{
	Super::BindToEnemy(InEnemyCharacter);

	if (!InEnemyCharacter)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		CachedPlayerCombatComponent = PlayerPawn->FindComponentByClass<UPlayerCombatComponent>();
		if (CachedPlayerCombatComponent.IsValid())
		{
			if (!CachedPlayerCombatComponent->OnLockOnTargetChanged.IsAlreadyBound(this, &ThisClass::OnPlayerLockOnChanged))
			{
				CachedPlayerCombatComponent->OnLockOnTargetChanged.AddDynamic(this, &ThisClass::OnPlayerLockOnChanged);
			}
		}
	}
}

void UCMEnemyHealthWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	if (CachedPlayerCombatComponent.IsValid())
	{
		if (CachedPlayerCombatComponent->OnLockOnTargetChanged.IsAlreadyBound(this, &ThisClass::OnPlayerLockOnChanged))
		{
			CachedPlayerCombatComponent->OnLockOnTargetChanged.RemoveDynamic(this, &ThisClass::OnPlayerLockOnChanged);
		}
	}

	Super::NativeDestruct();
}

void UCMEnemyHealthWidget::OnHealthChanged(float NewPercent)
{
	Super::OnHealthChanged(NewPercent);

	if (NewPercent > 0.0f)
	{
		bIsHealthChanged = true;
		UpdateHealthBarVisibility();

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				HideTimerHandle,
				this,
				&UCMEnemyHealthWidget::HideHealthBar,
				ShowDuration,
				false
			);
		}
	}
	else
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
		}
		bIsHealthChanged = false;
		bIsLockedOn = false;
		bIsCombatState = false;
		UpdateHealthBarVisibility();
	}
}

void UCMEnemyHealthWidget::OnPlayerLockOnChanged(bool bIsLockOn, AActor* TargetActor)
{
	if (bIsLockOn && TargetActor == CachedEnemyCharacter.Get())
	{
		bIsLockedOn = true;
	}
	else
	{
		bIsLockedOn = false;
	}

	UpdateHealthBarVisibility();
}

void UCMEnemyHealthWidget::UpdateHealthBarVisibility()
{
	Super::UpdateHealthBarVisibility();

	if (!CachedEnemyCharacter.IsValid() || bIsDead)
	{
		return;
	}

	if (bIsLockedOn || bIsCombatState || bIsHealthChanged)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UCMEnemyHealthWidget::HideHealthBar()
{
	bIsHealthChanged = false;
	UpdateHealthBarVisibility();
}