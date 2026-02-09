#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CMLockOnWidget.generated.h"

class UPlayerCombatComponent;

UCLASS()
class CRIMSONMOON_API UCMLockOnWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitLockOnWidget(UPlayerCombatComponent* CombatComponent);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void LockOnTargetChanged(bool bIsLockedOn, AActor* NewTarget);

private:
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTargetActor;

	bool bIsActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn", meta = (AllowPrivateAccess = "true"))
	float VerticalOffset;

	TWeakObjectPtr<UPlayerCombatComponent> CachedCombatComponent;
};
