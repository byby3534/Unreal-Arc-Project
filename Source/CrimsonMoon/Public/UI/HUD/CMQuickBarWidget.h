#pragma once

#include "CoreMinimal.h"
#include "UI/Core/CMBaseWidget.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "CMQuickBarWidget.generated.h"

class UCMQuickBarSlotWidget;

UCLASS()
class CRIMSONMOON_API UCMQuickBarWidget : public UCMBaseWidget
{
	GENERATED_BODY()

public:
	// 컴포넌트와 연결 및 초기 상태 동기화
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	void InitializeQuickBarComponent(UCMQuickBarComponent* InQuickBarComp);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnQuickBarSlotUpdated(int32 SlotIndex, UCMItemInstance* ItemInstance);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_WeaponSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_PotionSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCMQuickBarSlotWidget> WBP_UtilitySlot;

	UPROPERTY()
	TArray<TObjectPtr<UCMQuickBarSlotWidget>> QuickBarSlots;

private:
	TWeakObjectPtr<UCMQuickBarComponent> QuickBarComp;
};