#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMInteractionUIComponent.generated.h"

class UCMInteractionWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CRIMSONMOON_API UCMInteractionUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMInteractionUIComponent();

	UFUNCTION()
	void OnInteractableActorFound(AActor* NewTarget);

protected:
	virtual void InitializeComponent() override;

private:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ShowInteractionUI(const FInteractionUIData& Data);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateInteractionProgress(float CurrentProgress);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void HideInteractionUI();

	UPROPERTY(EditDefaultsOnly, Category = "UI|Interaction")
	TSubclassOf<UCMInteractionWidget> InteractionWidgetClass;

	UPROPERTY()
	TObjectPtr<UCMInteractionWidget> InteractionWidgetInstance;

	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayerController;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentInteractableActor;

	UFUNCTION()
	void OnTargetActorDestroyed(AActor* DestroyedActor);
};
