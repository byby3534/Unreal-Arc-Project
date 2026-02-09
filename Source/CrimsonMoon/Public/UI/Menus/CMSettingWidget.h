#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "CMSettingWidget.generated.h"

class UComboBoxString;
class UCMBaseButton;
class UTextBlock;
class UButton;
class USlider;

UCLASS()
class CRIMSONMOON_API UCMSettingWidget : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	UCMSettingWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> WindowModeComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> ResolutionComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> FrameRateComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> ShadowQualityComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> TextureQualityComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> ShaderQualityComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> AntiAliasingQualityComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> VSyncButtonPrev;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> VSyncButtonNext;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> VSyncTextValue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> BGMSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SFXSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBaseButton> SaveButton;

private:
	int32 PendingWindowModeIndex;
	int32 PendingResolutionIndex;
	int32 PendingFrameRateIndex;
	int32 PendingShadowQualityIndex;
	int32 PendingTextureQualityIndex;
	int32 PendingShaderQualityIndex;
	int32 PendingAntiAliasingQualityIndex;
	int32 PendingVSyncIndex;

	TArray<FIntPoint> SupportedResolutions;
	TArray<float> SupportedFrameRates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Config", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundMix> GlobalSoundMix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Config", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundClass> BGMClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Config", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundClass> SFXClass;

	UFUNCTION()
	void OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnResolutionChanged(FString SelectedIte, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnFrameRateChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnShadowQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnTextureQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnShaderQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnAntiAliasingQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnVSyncPrevClicked();

	UFUNCTION()
	void OnVSyncNextClicked();

	UFUNCTION()
	void OnBGMVolumeChanged(float Value);

	UFUNCTION()
	void OnSFXVolumeChanged(float Value);

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void OnSaveButtonClicked();

	void InitSettingDatas();
	void InitResolutionOptions();
	void InitFrameRateOptions();
	void UpdateVSyncText();
	void UpdateVolumeChanged(USoundClass* TargetClass, float Volume);
};
