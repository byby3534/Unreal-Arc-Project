#include "UI/HUD/CMLockOnWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Combat/PlayerCombatComponent.h"

void UCMLockOnWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
	bIsActive = false;
	VerticalOffset = 0.f;
}

void UCMLockOnWidget::InitLockOnWidget(UPlayerCombatComponent* CombatComponent)
{
	if (!IsValid(CombatComponent))
	{
		return;
	}

	CachedCombatComponent = CombatComponent;

	if (!CachedCombatComponent->OnLockOnTargetChanged.IsAlreadyBound(this, &ThisClass::LockOnTargetChanged))
	{
		CachedCombatComponent->OnLockOnTargetChanged.AddDynamic(this, &ThisClass::LockOnTargetChanged);
	}
}

void UCMLockOnWidget::NativeDestruct()
{
	if (CachedCombatComponent.IsValid())
	{
		if (CachedCombatComponent->OnLockOnTargetChanged.IsAlreadyBound(this, &ThisClass::LockOnTargetChanged))
		{
			CachedCombatComponent->OnLockOnTargetChanged.RemoveDynamic(this, &ThisClass::LockOnTargetChanged);
		}
	}
}

void UCMLockOnWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsActive || !CurrentTargetActor.IsValid())
	{
		if (GetVisibility() != ESlateVisibility::Hidden)
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	// 타겟 위치 계산
	FVector TargetLocation = CurrentTargetActor->GetActorLocation();
	// VerticalOffset만큼 높이 조절 가능
	TargetLocation.Z += VerticalOffset;

	// 월드 좌표 -> 화면 좌표 전환
	FVector2D ScreenPosition;
	bool bOnScreen = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		PC,
		TargetLocation,
		ScreenPosition,
		false
	);

	if (bOnScreen)
	{
		if (GetVisibility() != ESlateVisibility::HitTestInvisible)
		{
			SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		SetRenderTranslation(ScreenPosition);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UCMLockOnWidget::LockOnTargetChanged(bool bIsLockedOn, AActor* NewTarget)
{
	bIsActive = bIsLockedOn;
	CurrentTargetActor = NewTarget;

	if (bIsActive && CurrentTargetActor.IsValid())
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}