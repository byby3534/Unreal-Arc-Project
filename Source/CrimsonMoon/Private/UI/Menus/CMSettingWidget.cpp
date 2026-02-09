#include "UI/Menus/CMSettingWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "UI/Common/CMBaseButton.h"
#include "Components/ComboBoxString.h"
#include "Structs/CMStructTypes.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"

UCMSettingWidget::UCMSettingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetConfig.UILayer = EUILayerType::Popup;
	WidgetConfig.UIInputMode = EUIInputMode::UIOnly;
	WidgetConfig.bShowMouseCursor = true;
	WidgetConfig.bShouldCache = true;
	WidgetConfig.PreviousWidgetVisibility = ESlateVisibility::Hidden;

	PendingWindowModeIndex = 2; // 기본 값: 창 모드
	PendingResolutionIndex = 0;
	PendingFrameRateIndex = 0;
	PendingShadowQualityIndex = 2;
	PendingTextureQualityIndex = 2;
	PendingShaderQualityIndex = 2;
	PendingAntiAliasingQualityIndex = 2;
	PendingVSyncIndex = 0;
}

void UCMSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitResolutionOptions();
	InitFrameRateOptions();
	InitSettingDatas();

	if (GlobalSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(GetWorld(), GlobalSoundMix);
	}
}

void UCMSettingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 1. 윈도우 모드
	if (WindowModeComboBox)
	{
		WindowModeComboBox->OnSelectionChanged.AddDynamic(this, &UCMSettingWidget::OnWindowModeChanged);

		if (WindowModeComboBox->GetOptionCount() == 0)
		{
			WindowModeComboBox->AddOption(TEXT("전체 화면"));
			WindowModeComboBox->AddOption(TEXT("테두리 없는 창"));
			WindowModeComboBox->AddOption(TEXT("창 모드"));
		}
	}

	// 2. 해상도
	if (ResolutionComboBox)
	{
		ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &UCMSettingWidget::OnResolutionChanged);
	}

	// 3. 프레임
	if (FrameRateComboBox)
	{
		FrameRateComboBox->OnSelectionChanged.AddDynamic(this, &UCMSettingWidget::OnFrameRateChanged);
	}

	// 4. 그림자
	if (ShadowQualityComboBox)
	{
		ShadowQualityComboBox->OnSelectionChanged.AddDynamic(this, &UCMSettingWidget::OnShadowQualityChanged);

		if (ShadowQualityComboBox->GetOptionCount() == 0)
		{
			ShadowQualityComboBox->AddOption(TEXT("낮음"));
			ShadowQualityComboBox->AddOption(TEXT("중간"));
			ShadowQualityComboBox->AddOption(TEXT("높음"));
			ShadowQualityComboBox->AddOption(TEXT("에픽"));
		}
	}

	// 5. 텍스처
	if (TextureQualityComboBox)
	{
		TextureQualityComboBox->OnSelectionChanged.AddDynamic(this, &UCMSettingWidget::OnTextureQualityChanged);
		if (TextureQualityComboBox->GetOptionCount() == 0)
		{
			TextureQualityComboBox->AddOption(TEXT("낮음"));
			TextureQualityComboBox->AddOption(TEXT("중간"));
			TextureQualityComboBox->AddOption(TEXT("높음"));
			TextureQualityComboBox->AddOption(TEXT("에픽"));
		}
	}

	// 6. 쉐이더
	if (ShaderQualityComboBox)
	{
		ShaderQualityComboBox->OnSelectionChanged.AddDynamic(this, &UCMSettingWidget::OnShaderQualityChanged);
		if (ShaderQualityComboBox->GetOptionCount() == 0)
		{
			ShaderQualityComboBox->AddOption(TEXT("낮음"));
			ShaderQualityComboBox->AddOption(TEXT("중간"));
			ShaderQualityComboBox->AddOption(TEXT("높음"));
			ShaderQualityComboBox->AddOption(TEXT("에픽"));
		}
	}

	// 7. 안티 앨리어싱
	if (AntiAliasingQualityComboBox)
	{
		AntiAliasingQualityComboBox->OnSelectionChanged.AddDynamic(this, &UCMSettingWidget::OnAntiAliasingQualityChanged);
		if (AntiAliasingQualityComboBox->GetOptionCount() == 0)
		{
			AntiAliasingQualityComboBox->AddOption(TEXT("낮음"));
			AntiAliasingQualityComboBox->AddOption(TEXT("중간"));
			AntiAliasingQualityComboBox->AddOption(TEXT("높음"));
			AntiAliasingQualityComboBox->AddOption(TEXT("에픽"));
		}
	}

	// 8. Vsync
	if (VSyncButtonPrev)
	{
		VSyncButtonPrev->OnClicked.AddDynamic(this, &UCMSettingWidget::OnVSyncPrevClicked);
	}

	if (VSyncButtonNext)
	{
		VSyncButtonNext->OnClicked.AddDynamic(this, &UCMSettingWidget::OnVSyncNextClicked);
	}

	if (BGMSlider)
	{
		BGMSlider->SetValue(1.0f);
		BGMSlider->OnValueChanged.AddDynamic(this, &UCMSettingWidget::OnBGMVolumeChanged);
	}

	if (SFXSlider)
	{
		SFXSlider->SetValue(1.0f);
		SFXSlider->OnValueChanged.AddDynamic(this, &UCMSettingWidget::OnSFXVolumeChanged);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &ThisClass::OnCloseButtonClicked);
		CloseButton->SetButtonText(NSLOCTEXT("MenuWidget", "CloseButton", "닫기"));
	}

	if (SaveButton)
	{
		SaveButton->OnClicked.AddDynamic(this, &ThisClass::OnSaveButtonClicked);
		SaveButton->SetButtonText(NSLOCTEXT("MenuWidget", "CloseButton", "저장"));
	}
}

void UCMSettingWidget::OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (WindowModeComboBox)
	{	
		PendingWindowModeIndex = WindowModeComboBox->GetSelectedIndex();
	}
}

void UCMSettingWidget::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (ResolutionComboBox)
	{
		PendingResolutionIndex = ResolutionComboBox->GetSelectedIndex();
	}
}

void UCMSettingWidget::OnFrameRateChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (FrameRateComboBox)
	{
		PendingFrameRateIndex = FrameRateComboBox->GetSelectedIndex();
	}
}

void UCMSettingWidget::OnShadowQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (ShadowQualityComboBox)
	{
		PendingShadowQualityIndex = ShadowQualityComboBox->GetSelectedIndex();
	}
}

void UCMSettingWidget::OnTextureQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (TextureQualityComboBox)
	{
		PendingTextureQualityIndex = TextureQualityComboBox->GetSelectedIndex();
	}
}

void UCMSettingWidget::OnShaderQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (ShaderQualityComboBox)
	{
		PendingShaderQualityIndex = ShaderQualityComboBox->GetSelectedIndex();
	}
}

void UCMSettingWidget::OnAntiAliasingQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (AntiAliasingQualityComboBox)
	{
		PendingAntiAliasingQualityIndex = AntiAliasingQualityComboBox->GetSelectedIndex();
	}
}

void UCMSettingWidget::OnVSyncPrevClicked()
{
	PendingVSyncIndex = (PendingVSyncIndex == 0) ? 1 : 0;
	UpdateVSyncText();
}

void UCMSettingWidget::OnVSyncNextClicked()
{
	PendingVSyncIndex = (PendingVSyncIndex == 0) ? 1 : 0;
	UpdateVSyncText();
}

void UCMSettingWidget::OnBGMVolumeChanged(float Value)
{
	UpdateVolumeChanged(BGMClass, Value);
}

void UCMSettingWidget::OnSFXVolumeChanged(float Value)
{
	UpdateVolumeChanged(SFXClass, Value);
}

void UCMSettingWidget::OnCloseButtonClicked()
{
	OnBackAction();
}

void UCMSettingWidget::OnSaveButtonClicked()
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings)
		return;

	// 1. 윈도우 모드
	EWindowMode::Type NewMode = EWindowMode::Windowed;
	switch (PendingWindowModeIndex)
	{
		case 0:
			NewMode = EWindowMode::Fullscreen;
			break;
		case 1:
			NewMode = EWindowMode::WindowedFullscreen;
			break;
		case 2:
			NewMode = EWindowMode::Windowed;
			break;
	}
	UserSettings->SetFullscreenMode(NewMode);

	// 2. 해상도
	if (SupportedResolutions.IsValidIndex(PendingResolutionIndex))
	{
		FIntPoint NewRes = SupportedResolutions[PendingResolutionIndex];
		UserSettings->SetScreenResolution(NewRes);
	}

	// 3. 프레임
	if (SupportedFrameRates.IsValidIndex(PendingFrameRateIndex))
	{
		UserSettings->SetFrameRateLimit(SupportedFrameRates[PendingFrameRateIndex]);
	}

	// 4. 그림자
	UserSettings->SetShadowQuality(PendingShadowQualityIndex);

	// 5. 텍스처
	UserSettings->SetTextureQuality(PendingTextureQualityIndex);

	// 6. 쉐이더
	UserSettings->SetShadingQuality(PendingShaderQualityIndex);

	// 7. 안티 앨리어싱
	UserSettings->SetAntiAliasingQuality(PendingAntiAliasingQualityIndex);

	// 8. Vsync
	UserSettings->SetVSyncEnabled(PendingVSyncIndex == 1);

	UserSettings->ApplySettings(false);
}

void UCMSettingWidget::InitSettingDatas()
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	// 1. 윈도우 모드
	EWindowMode::Type CurrentMode = UserSettings->GetFullscreenMode();

	switch (CurrentMode)
	{
		case EWindowMode::Fullscreen:
			PendingWindowModeIndex = 0;
			break;
		case EWindowMode::WindowedFullscreen:
			PendingWindowModeIndex = 1;
			break;
		case EWindowMode::Windowed:
			PendingWindowModeIndex = 2;
			break;
		default:
			PendingWindowModeIndex = 2;
	}

	if (WindowModeComboBox)
	{
		WindowModeComboBox->SetSelectedIndex(PendingWindowModeIndex);
	}

	// 2. 해상도
	FIntPoint CurrentRes = UserSettings->GetScreenResolution();
	PendingResolutionIndex = 0;

	for (int32 i = 0; i < SupportedResolutions.Num(); ++i)
	{
		if (SupportedResolutions[i] == CurrentRes)
		{
			PendingResolutionIndex = i;
			break;
		}
	}

	if (PendingResolutionIndex == 0 && SupportedResolutions.Num() > 0 && SupportedResolutions[0] != CurrentRes)
	{
		if (SupportedResolutions.IsValidIndex(2))
		{
			PendingResolutionIndex = 2;
		}
	}

	if (ResolutionComboBox)
	{
		ResolutionComboBox->SetSelectedIndex(PendingResolutionIndex);
	}

	float CurrentFrameRate = UserSettings->GetFrameRateLimit();
	PendingFrameRateIndex = 0;

	bool bFound = false;
	for (int32 i = 0; i < SupportedFrameRates.Num(); ++i)
	{
		if (FMath::IsNearlyEqual(SupportedFrameRates[i], CurrentFrameRate, 0.1f))
		{
			PendingFrameRateIndex = i;
			bFound = true;
			break;
		}
	}

	if (!bFound && SupportedFrameRates.Num() > 0)
	{
		PendingFrameRateIndex = SupportedFrameRates.Num() - 1;
	}

	if (FrameRateComboBox)
	{
		FrameRateComboBox->SetSelectedIndex(PendingFrameRateIndex);
	}

	// 3. 그림자
	int32 CurrentShadow = UserSettings->GetShadowQuality();
	PendingShadowQualityIndex = FMath::Clamp(CurrentShadow, 0, 3);

	if (ShadowQualityComboBox)
	{
		ShadowQualityComboBox->SetSelectedIndex(PendingShadowQualityIndex);
	}

	// 4. 텍스처
	int32 CurrentTexture = UserSettings->GetTextureQuality();
	PendingTextureQualityIndex = FMath::Clamp(CurrentTexture, 0, 3);
	if (TextureQualityComboBox)
	{
		TextureQualityComboBox->SetSelectedIndex(PendingTextureQualityIndex);
	}

	// 5. 쉐이더
	int32 CurrentShading = UserSettings->GetShadingQuality();
	PendingShaderQualityIndex = FMath::Clamp(CurrentShading, 0, 3);
	if (ShaderQualityComboBox)
	{
		ShaderQualityComboBox->SetSelectedIndex(PendingShaderQualityIndex);
	}

	// 6. 안티 앨리어싱
	int32 CurrentAA = UserSettings->GetAntiAliasingQuality();
	PendingAntiAliasingQualityIndex = FMath::Clamp(CurrentAA, 0, 3);
	if (AntiAliasingQualityComboBox)
	{
		AntiAliasingQualityComboBox->SetSelectedIndex(PendingAntiAliasingQualityIndex);
	}

	// 7. Vsync
	bool bIsVSync = UserSettings->IsVSyncEnabled();
	PendingVSyncIndex = bIsVSync ? 1 : 0;
	UpdateVSyncText();
}

void UCMSettingWidget::InitResolutionOptions()
{
	SupportedResolutions.Empty();
	if (ResolutionComboBox)
	{
		ResolutionComboBox->ClearOptions();
	}

	FIntPoint DesktopResolution = FIntPoint::ZeroValue;
	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		DesktopResolution = UserSettings->GetDesktopResolution();
	}

	TArray<FIntPoint> Candidates;
	Candidates.Add(FIntPoint(1280, 720));
	Candidates.Add(FIntPoint(1600, 900));
	Candidates.Add(FIntPoint(1920, 1080));
	Candidates.Add(FIntPoint(2560, 1440));
	Candidates.Add(FIntPoint(3840, 2160));

	for (const FIntPoint& Candidate : Candidates)
	{
		if (DesktopResolution == FIntPoint::ZeroValue || (Candidate.X <= DesktopResolution.X && Candidate.Y <= DesktopResolution.Y))
		{
			SupportedResolutions.Add(Candidate);

			if (ResolutionComboBox)
			{
				FString OptionStr = FString::Printf(TEXT("%d x %d"), Candidate.X, Candidate.Y);
				ResolutionComboBox->AddOption(OptionStr);
			}
		}
	}
}

void UCMSettingWidget::InitFrameRateOptions()
{
	SupportedFrameRates.Empty();
	if (FrameRateComboBox)
	{
		FrameRateComboBox->ClearOptions();
	}

	SupportedFrameRates.Add(30.0f);
	SupportedFrameRates.Add(60.0f);
	SupportedFrameRates.Add(120.0f);
	SupportedFrameRates.Add(144.0f);
	SupportedFrameRates.Add(240.0f);
	SupportedFrameRates.Add(0.0f);

	if (FrameRateComboBox)
	{
		for (float Rate : SupportedFrameRates)
		{
			if (Rate == 0.0f)
			{
				FrameRateComboBox->AddOption(TEXT("무제한"));
			}
			else
			{
				FrameRateComboBox->AddOption(FString::Printf(TEXT("%d FPS"), (int32)Rate));
			}
		}
	}
}

void UCMSettingWidget::UpdateVSyncText()
{
	if (VSyncTextValue)
	{
		FString TextStr = (PendingVSyncIndex == 1) ? TEXT("켜기 (On)") : TEXT("끄기 (Off)");
		VSyncTextValue->SetText(FText::FromString(TextStr));
	}
}

void UCMSettingWidget::UpdateVolumeChanged(USoundClass* TargetClass, float Volume)
{
	if (!TargetClass || !GlobalSoundMix)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float SafeVolume = (Volume < KINDA_SMALL_NUMBER) ? 0.0001f : Volume;

	if (GlobalSoundMix && TargetClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			GlobalSoundMix,
			TargetClass,
			SafeVolume,
			1.0f,
			0.0f,
			true
		);
	}
}
