#pragma once

#include "CoreMinimal.h"
#include "UI/HUD/Enemy/CMEnemyHealthWidgetBase.h"
#include "CMEnemyHealthWidget.generated.h"

class UProgressBar;
class UPawnUIComponent;
class ACMEnemyCharacterBase;
class UPlayerCombatComponent;

UCLASS()
class CRIMSONMOON_API UCMEnemyHealthWidget : public UCMEnemyHealthWidgetBase
{
	GENERATED_BODY()

public:
	UCMEnemyHealthWidget(const FObjectInitializer& ObjectInitializer);

	virtual void BindToEnemy(ACMEnemyCharacterBase* InEnemyCharacter) override;

protected:
	virtual void NativeDestruct() override;

	virtual void OnHealthChanged(float NewPercent) override;

	virtual void UpdateHealthBarVisibility() override;

	UFUNCTION()
	void OnPlayerLockOnChanged(bool bIsLockedOn, AActor* TargetActor);

private:
	bool bIsLockedOn;
	bool bIsHealthChanged;

	FTimerHandle HideTimerHandle;
	float ShowDuration;
	void HideHealthBar();

	TWeakObjectPtr<UPlayerCombatComponent> CachedPlayerCombatComponent;
};
