#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enums/CMEnums.h"
#include "Structs/CMStructTypes.h"
#include "CMBaseWidget.generated.h"

class UUIManagerComponent;

UCLASS()
class CRIMSONMOON_API UCMBaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCMBaseWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Widget|UI Config")
	FWidgetConfig WidgetConfig;

	UFUNCTION(BlueprintPure, Category = "Base Widget|Cache")
	APlayerController* GetPlayerController() const { return CachedPlayerController.Get(); }

	UFUNCTION(BlueprintPure, Category = "Base Widget|Cache")
	APlayerState* GetCachedPlayerState();

	UFUNCTION(BlueprintPure, Category = "Base Widget|Cache")
	UGameInstance* GetCachedGameInstance() const { return CachedGameInstance.Get(); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Widget|Cache")
	TWeakObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Widget|Cache")
	TWeakObjectPtr<APlayerState> CachedPlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Widget|Cache")
	TWeakObjectPtr<UGameInstance> CachedGameInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Widget|Sound")
	USoundBase* OpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Widget|Sound")
	USoundBase* CloseSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Widget|Cache")
	TObjectPtr<UUIManagerComponent> UIManager;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Base Widget|Sound")
	void PlayOpenWidget();

	UFUNCTION(BlueprintCallable, Category = "Base Widget|Sound")
	void PlayCloseWidget();

	UFUNCTION(BlueprintCallable, Category = "Base Widget|InputAction")
	virtual void OnBackAction();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

};
