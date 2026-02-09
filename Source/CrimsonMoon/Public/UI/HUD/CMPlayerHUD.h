#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "CMPlayerHUD.generated.h"

class UCMPlayerStatusWidget;
class UCMLockOnWidget;
class UCMSkillSlotWidget;
class UCMBossHealthWidget;
class ACMEnemyCharacterBase;
class UCMQuickBarWidget;
class UCMQuickBarComponent;

UCLASS()
class CRIMSONMOON_API UCMPlayerHUD : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	UCMPlayerHUD(const FObjectInitializer& ObjectInitializer);

	void ToggleHelpWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitBossHealthBar();

	void OnGameStateSet(AGameStateBase* NewGameState);

	UFUNCTION()
	void OnAbilityIconSlotLoaded(FAbilityIconData InAbilityData);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMPlayerStatusWidget> PlayerStatusWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMLockOnWidget> LockOnWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMBossHealthWidget> BossHealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMSkillSlotWidget> SkillSlot_Q;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMSkillSlotWidget> SkillSlot_E;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMSkillSlotWidget> SkillSlot_R;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMSkillSlotWidget> SkillSlot_F;

	UPROPERTY(meta = (BindWidget))
	UCMQuickBarWidget* QuickBarWidget;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UCMSkillSlotWidget>> SkillSlots;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> HelpWidget;

private:
	UFUNCTION()
	void InitBossHealthWidget(ACMEnemyCharacterBase* NewBoss);

	FTimerHandle GameStateInitTimerHandle;
};