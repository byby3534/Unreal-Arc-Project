#pragma once

#include "CoreMinimal.h"
#include "UI/HUD/Enemy/CMEnemyHealthWidgetBase.h"
#include "CMBossHealthWidget.generated.h"

class UTextBlock;

UCLASS()
class CRIMSONMOON_API UCMBossHealthWidget : public UCMEnemyHealthWidgetBase
{
	GENERATED_BODY()

public:
	virtual void BindToEnemy(ACMEnemyCharacterBase* InEnemyCharacter) override;

protected:
	virtual void UpdateHealthBarVisibility() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BossName;
};
