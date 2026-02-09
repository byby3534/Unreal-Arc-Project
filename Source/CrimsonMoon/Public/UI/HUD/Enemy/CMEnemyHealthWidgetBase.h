#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CMEnemyHealthWidgetBase.generated.h"

class ACMEnemyCharacterBase;
class UPlayerCombatComponent;
class UPawnUIComponent;
class UProgressBar;

UCLASS()
class CRIMSONMOON_API UCMEnemyHealthWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UCMEnemyHealthWidgetBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "UI")
	virtual void BindToEnemy(ACMEnemyCharacterBase* InEnemyCharacter);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UFUNCTION()
	virtual void OnHealthChanged(float NewPercent);
	
	UFUNCTION()
	virtual void OnEnemyStateChanged(ECMEnemyState NewState);

	UFUNCTION()
	virtual void UpdateHealthBarVisibility();

	TWeakObjectPtr<ACMEnemyCharacterBase> CachedEnemyCharacter;

	TWeakObjectPtr<UPawnUIComponent> CachedPawnUIComponent;

	bool bIsCombatState;
	bool bIsDead;
};
