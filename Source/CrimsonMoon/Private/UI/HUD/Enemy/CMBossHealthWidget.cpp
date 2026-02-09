#include "UI/HUD/Enemy/CMBossHealthWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"

void UCMBossHealthWidget::BindToEnemy(ACMEnemyCharacterBase* InEnemyCharacter)
{
	Super::BindToEnemy(InEnemyCharacter);

	if (InEnemyCharacter)
	{
		BossName->SetText(InEnemyCharacter->GetEnemyName());
	}
}

void UCMBossHealthWidget::UpdateHealthBarVisibility()
{
	Super::UpdateHealthBarVisibility();

	if (bIsCombatState)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}