#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/CMStructTypes.h"
#include "Enums/CMEnums.h"
#include "UIManagerComponent.generated.h"

class UCMInputComponent;
class UCMDataAsset_InputConfig;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CRIMSONMOON_API UUIManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUIManagerComponent();

protected:
	virtual void InitializeComponent() override;

#pragma region HUD
public:
	UFUNCTION(BlueprintCallable, Category = "UI|HUD")
	void ShowHUD(TSubclassOf<UUserWidget> HUDClass);

	UFUNCTION(BlueprintCallable, Category = "UI|HUD")
	void HideHUD();

protected:
	UPROPERTY(VisibleAnywhere, Category = "UI|HUD")
	TObjectPtr<UUserWidget> ActiveHUDWidget;

	int32 GetZOrderForLayer(EUILayerType Layer) const;
#pragma endregion

#pragma region UIStack
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* PushWidget(TSubclassOf<UUserWidget> WidgetClass, EUILayerType Layer = EUILayerType::None);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void PopWidget(bool bShowPrevious = true);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* GetTopWidget() const;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnBackAction();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ResetUI();

protected:
	void UpdateInputMode();

	UPROPERTY(VisibleAnywhere, Category = "UI Stack")
	TArray<TObjectPtr<UUserWidget>> UIStack;

	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> CachedWidgets;

	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayerController;
#pragma endregion
};
