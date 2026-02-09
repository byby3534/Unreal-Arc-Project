#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CMPlayerStatusWidget.generated.h"

class UPawnUIComponent;
class UProgressBar;

UCLASS()
class CRIMSONMOON_API UCMPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitPlayerStatus(UPawnUIComponent* PawnUIComponent);

	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ManaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

private:
	TWeakObjectPtr<UPawnUIComponent> CachedPawnUIComponent;

	UFUNCTION()
	void OnHealthChanged(float NewPercent);

	UFUNCTION()
	void OnManaChanged(float NewPercent);

	UFUNCTION()
	void OnStaminaChanged(float NewPercent);
};
