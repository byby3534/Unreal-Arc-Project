// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMInventoryUIComponent.generated.h"

class UCMInventoryWidget;
class UCMInventoryComponent;
class UCMItemInstance;
class UUIManagerComponent;
class UCMQuickBarComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CRIMSONMOON_API UCMInventoryUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMInventoryUIComponent();

	void InitializeInventoryUI(UCMInventoryComponent* InInventoryComponent, UCMQuickBarComponent* InQuickBarComponent);

	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void OpenInventoryUI();

protected:
	virtual void InitializeComponent() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI|Inventory")
	TSubclassOf<UCMInventoryWidget> InventoryWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UCMInventoryComponent> CachedInventoryComponent;

	UPROPERTY(Transient)
	TObjectPtr<UCMQuickBarComponent> CachedQuickBarComponent;

	UPROPERTY(Transient)
	TObjectPtr<UUIManagerComponent> UIManager;
};
