#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "CMSkillSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UCMAbilitySystemComponent;

UCLASS()
class CRIMSONMOON_API UCMSkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SkillSlot")
	void InitASC(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "SkillSlot")
	void InitSkillSlotIcon(FAbilityIconData InAbilityData);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
 	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SkillIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CooldownImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> DimmedIcon;
private:
	FGameplayTag CachedInputTag;
	FKey GetKeyFromInputTag(const FGameplayTag& InputTag) const;

	UFUNCTION()
	void OnControlMapped();

	void OnCooldownChanged(const FGameplayTag CallbackTag, int32 NewCount);

	void LoadIconAsync(const TSoftObjectPtr<UTexture2D>& SoftIcon);

	TWeakObjectPtr<UCMAbilitySystemComponent> AbilitySystemComponent;
	FGameplayAbilitySpecHandle AbilitySpecHandle;
	TSharedPtr<struct FStreamableHandle> IconLoadHandle;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> CooldownMaterialDynamic;

	FGameplayTag CachedCooldownTag;
	bool bIsCooldownActive;
	FDelegateHandle CooldownTagDelegateHandle;

	void BindCooldownTag();
	void UpdateCoolTime();
	void UpdateCanUseSkill();
};
