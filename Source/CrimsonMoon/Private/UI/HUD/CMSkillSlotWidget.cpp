#include "UI/HUD/CMSkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySystem/Abilities/Player/CMPlayerGameplayAbility.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "DataAssets/Input/CMDataAsset_InputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Engine/AssetManager.h"

void UCMSkillSlotWidget::InitASC(UAbilitySystemComponent* ASC)
{
	AbilitySystemComponent = Cast<UCMAbilitySystemComponent>(ASC);
}

void UCMSkillSlotWidget::InitSkillSlotIcon(FAbilityIconData InAbilityData)
{
	if (AbilitySystemComponent.IsValid() && CachedCooldownTag.IsValid() && CooldownTagDelegateHandle.IsValid())
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(CachedCooldownTag, EGameplayTagEventType::NewOrRemoved).Remove(CooldownTagDelegateHandle);
		CooldownTagDelegateHandle.Reset();
	}

	bIsCooldownActive = false;

	if (CooldownImage)
	{
		CooldownMaterialDynamic = CooldownImage->GetDynamicMaterial();
		CooldownImage->SetVisibility(ESlateVisibility::Hidden);

		if (CooldownMaterialDynamic)
		{
			CooldownMaterialDynamic->SetScalarParameterValue(FName("Percent"), 0.0f);
		}
	}

	LoadIconAsync(InAbilityData.Icon);

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}

	CachedCooldownTag = InAbilityData.CooldownTag;
	if (CachedCooldownTag.IsValid())
	{
		BindCooldownTag();
	}

	CachedInputTag = InAbilityData.InputTag;
	OnControlMapped();

	if (AbilitySystemComponent.IsValid())
	{
		FGameplayAbilitySpec* FoundSpec = nullptr;
		for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (Spec.Ability->GetAssetTags().HasTag(CachedInputTag))
			{
				FoundSpec = (FGameplayAbilitySpec*)&Spec;
				break;
			}
		}

		if (FoundSpec)
		{
			AbilitySpecHandle = FoundSpec->Handle;
		}
		else
		{
			AbilitySpecHandle = FGameplayAbilitySpecHandle();

			if (DimmedIcon)
			{
				DimmedIcon->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void UCMSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsCooldownActive = false;

	if (SkillIcon)
	{
		SkillIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (DimmedIcon)
	{
		DimmedIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}

	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* SubSystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				SubSystem->ControlMappingsRebuiltDelegate.RemoveDynamic(this, &ThisClass::OnControlMapped);
				SubSystem->ControlMappingsRebuiltDelegate.AddDynamic(this, &ThisClass::OnControlMapped);
			}
		}
	}
}

void UCMSkillSlotWidget::NativeDestruct()
{
	if (AbilitySystemComponent.IsValid() && CachedCooldownTag.IsValid() && CooldownTagDelegateHandle.IsValid())
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(CachedCooldownTag, EGameplayTagEventType::NewOrRemoved).Remove(CooldownTagDelegateHandle);
		CooldownTagDelegateHandle.Reset();
	}

	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* SubSystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				SubSystem->ControlMappingsRebuiltDelegate.RemoveDynamic(this, &ThisClass::OnControlMapped);
			}
		}
	}

	Super::NativeDestruct();
}

void UCMSkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateCoolTime();

	UpdateCanUseSkill();
}

void UCMSkillSlotWidget::BindCooldownTag()
{
	if (!AbilitySystemComponent.IsValid() || !CachedCooldownTag.IsValid())
	{
		return;
	}

	if (CooldownTagDelegateHandle.IsValid())
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(CachedCooldownTag, EGameplayTagEventType::NewOrRemoved).Remove(CooldownTagDelegateHandle);
		CooldownTagDelegateHandle.Reset();
	}

	CooldownTagDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(CachedCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnCooldownChanged);

	const int32 Count = AbilitySystemComponent->GetTagCount(CachedCooldownTag);
	OnCooldownChanged(CachedCooldownTag, Count);
}

FKey UCMSkillSlotWidget::GetKeyFromInputTag(const FGameplayTag& InputTag) const
{
	const APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return EKeys::Invalid;
	}

	const ACMPlayerCharacterBase* Character = Cast<ACMPlayerCharacterBase>(PC->GetPawn());
	if (!Character)
	{
		return EKeys::Invalid;
	}

	const UCMDataAsset_InputConfig* InputConfig = Character->GetInputConfigDataAsset();
	if (!InputConfig)
	{
		return EKeys::Invalid;
	}

	const UInputAction* FoundAction = InputConfig->FindAbilityInputActionByTag(InputTag);
	if (!FoundAction)
	{
		return EKeys::Invalid;
	}

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return EKeys::Invalid;
	}

	const UEnhancedInputLocalPlayerSubsystem* SubSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!SubSystem)
	{
		return EKeys::Invalid;
	}

	TArray<FKey> Keys = SubSystem->QueryKeysMappedToAction(FoundAction);

	if (Keys.Num() > 0)
	{
		return Keys[0];
	}

	return EKeys::Invalid;
}

void UCMSkillSlotWidget::OnControlMapped()
{
	if (!KeyText)
	{
		return;
	}

	if (!CachedInputTag.IsValid())
	{
		KeyText->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	FKey MappedKey = GetKeyFromInputTag(CachedInputTag);

	if (MappedKey.IsValid())
	{
		KeyText->SetText(MappedKey.GetDisplayName());
		KeyText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		KeyText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UCMSkillSlotWidget::OnCooldownChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsCooldownActive = (NewCount > 0);

	if (bIsCooldownActive)
	{
		if (CooldownImage)
		{
			CooldownImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	else
	{
		if (CooldownMaterialDynamic)
		{
			CooldownMaterialDynamic->SetScalarParameterValue(FName("Percent"), 0.0f);
		}

		if (CooldownImage)
		{
			CooldownImage->SetVisibility(ESlateVisibility::Hidden);
		}

		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility(ESlateVisibility::Hidden));
		}
	}
}

void UCMSkillSlotWidget::LoadIconAsync(const TSoftObjectPtr<UTexture2D>& SoftIcon)
{
	if (IconLoadHandle.IsValid())
	{
		IconLoadHandle->CancelHandle();
		IconLoadHandle.Reset();
	}

	if (SoftIcon.IsNull())
	{
		if (SkillIcon)
		{
			SkillIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	if (UTexture2D* CachedTexture = SoftIcon.Get())
	{
		if (SkillIcon)
		{
			SkillIcon->SetBrushFromTexture(CachedTexture);
			SkillIcon->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	if (SkillIcon)
	{
		SkillIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	IconLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SoftIcon.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, SoftIcon]() {

			if (!IsValid(this))
			{
				return;
			}

			if (UTexture2D* LoadedTexture = SoftIcon.Get())
			{
				if (SkillIcon)
				{
					SkillIcon->SetBrushFromTexture(LoadedTexture);
					SkillIcon->SetVisibility(ESlateVisibility::Visible);
				}
			}

			IconLoadHandle.Reset();
		})
	);
}

void UCMSkillSlotWidget::UpdateCoolTime()
{
	if (!bIsCooldownActive)
	{
		return;
	}

	if (!AbilitySystemComponent.IsValid() || !CachedCooldownTag.IsValid())
	{
		return;
	}

	// CooldownTag가 있는 GE 쿼리를 저장
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CachedCooldownTag));

	// TimeRemainingAndDuration.Key - 남은 지속 시간
	// TimeRemainingAndDuration.Value - 전체 지속 시간
	TArray<TPair<float, float>> TimeRemainingAndDuration = AbilitySystemComponent->GetActiveEffectsTimeRemainingAndDuration(Query);

	if (TimeRemainingAndDuration.Num() > 0)
	{
		float TimeRemaining = TimeRemainingAndDuration[0].Key;
		float CoolDownDuration = TimeRemainingAndDuration[0].Value;

		if (CooldownMaterialDynamic)
		{
			CooldownMaterialDynamic->SetScalarParameterValue(FName("Percent"), (TimeRemaining / CoolDownDuration));
		}

		if (CooldownText)
		{
			int32 IntTimeRemaining = FMath::CeilToInt(TimeRemaining);
			if (IntTimeRemaining > 0)
			{
				CooldownText->SetVisibility(ESlateVisibility::Visible);
				CooldownText->SetText(FText::AsNumber(IntTimeRemaining));
			}
			else
			{
				CooldownText->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void UCMSkillSlotWidget::UpdateCanUseSkill()
{
	if (bIsCooldownActive)
	{
		if (DimmedIcon)
		{
			DimmedIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (!AbilitySystemComponent.IsValid() || !AbilitySpecHandle.IsValid())
	{
		return;
	}

	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (!Spec || !Spec->Ability)
	{
		return;
	}

	UGameplayAbility* AbilityCDO = Spec->Ability;

	FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();

	if (AbilityCDO->CanActivateAbility(AbilitySpecHandle, ActorInfo, nullptr, nullptr, nullptr))
	{
		if (DimmedIcon)
		{
			DimmedIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		if (DimmedIcon)
		{
			DimmedIcon->SetVisibility(ESlateVisibility::Visible);
		}
	}
}