#include "UI/HUD/Enemy/CMEnemyHealthWidgetBase.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "Components/UI/UPawnUIComponent.h"
#include "Components/ProgressBar.h"

UCMEnemyHealthWidgetBase::UCMEnemyHealthWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsCombatState = false;
	bIsDead = false;
}

void UCMEnemyHealthWidgetBase::BindToEnemy(ACMEnemyCharacterBase* InEnemyCharacter)
{
	if (!IsValid(InEnemyCharacter))
	{
		return;
	}

	CachedPawnUIComponent = InEnemyCharacter->GetPawnUIComponent();
	CachedEnemyCharacter = InEnemyCharacter;

	// 체력 변경 델리게이트
	if (CachedPawnUIComponent.IsValid())
	{
		if (!CachedPawnUIComponent->OnCurrentHealthChanged.IsAlreadyBound(this, &ThisClass::OnHealthChanged))
		{
			CachedPawnUIComponent->OnCurrentHealthChanged.AddDynamic(this, &ThisClass::OnHealthChanged);
			CachedPawnUIComponent->BroadcastInitStatusValues();
		}
	}

	// 적 상태 델리게이트
	if (CachedEnemyCharacter.IsValid())
	{
		if (!CachedEnemyCharacter->OnEnemyStateChanged.IsAlreadyBound(this, &ThisClass::OnEnemyStateChanged))
		{
			CachedEnemyCharacter->OnEnemyStateChanged.AddDynamic(this, &ThisClass::OnEnemyStateChanged);
		}

		OnEnemyStateChanged(CachedEnemyCharacter->GetAIState());
	}
}

void UCMEnemyHealthWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
}

void UCMEnemyHealthWidgetBase::NativeDestruct()
{
	if (CachedPawnUIComponent.IsValid())
	{
		if (CachedPawnUIComponent->OnCurrentHealthChanged.IsAlreadyBound(this, &ThisClass::OnHealthChanged))
		{
			CachedPawnUIComponent->OnCurrentHealthChanged.RemoveDynamic(this, &ThisClass::OnHealthChanged);
		}
	}

	if (CachedEnemyCharacter.IsValid())
	{
		if (CachedEnemyCharacter->OnEnemyStateChanged.IsAlreadyBound(this, &ThisClass::OnEnemyStateChanged))
		{
			CachedEnemyCharacter->OnEnemyStateChanged.RemoveDynamic(this, &ThisClass::OnEnemyStateChanged);
		}
	}

	Super::NativeDestruct();
}

void UCMEnemyHealthWidgetBase::OnHealthChanged(float NewPercent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(NewPercent);
	}
}

void UCMEnemyHealthWidgetBase::OnEnemyStateChanged(ECMEnemyState NewState)
{
	bIsCombatState = (NewState == ECMEnemyState::Combat);
	bIsDead = (NewState == ECMEnemyState::Dead);

	UpdateHealthBarVisibility();
}

void UCMEnemyHealthWidgetBase::UpdateHealthBarVisibility()
{
	if (!CachedEnemyCharacter.IsValid() || bIsDead)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}
}
