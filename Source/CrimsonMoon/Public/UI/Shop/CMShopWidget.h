// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Npc/CMNpcBase.h"
#include "Structs/CMShopTypes.h"
#include "UI/Core/CMBaseWidget.h"
class UTextBlock;
class UButton;
struct FCMShopItemContent;
class UVerticalBox;
class UCMShopContentElementWidget;
class ACMPlayerController;
#include "CMShopWidget.generated.h"

/**
 * 
 */

UCLASS()
class CRIMSONMOON_API UCMShopWidget : public UCMBaseWidget
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	UCMShopWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

	/* Custom Methods */
public:
	/* Function */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void BuildShopList();

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetShopItems(const TArray<FCMShopItemContent>& InItems);

	UFUNCTION()
	void HandleElementSelected(UCMShopContentElementWidget* ElementWidget);

protected:
	/* Property */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ShopItemListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SellButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AddItemQuantityButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SubtractItemQuantityButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedItemQuantityText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultMessageText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TSubclassOf<UCMShopContentElementWidget> ShopItemWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TArray<FCMShopItemContent> ShopItems;

	// 구매/판매 결과 피드백 (블루프린트에서 확장 가능)
	UFUNCTION(BlueprintNativeEvent, Category = "Shop|Feedback")
	void OnPurchaseResultReceived(const FPurchaseResponse& Response, bool bIsBuy);

	virtual void NativeDestruct() override;

private:
	/* Variable */
	FCMShopItemContent CurrentSelectedItem;
	int32 SelectedItemQuantity = 0;

	/* Function */
	void UpdateSelectedItemDisplay();
	void UpdateSelectedItemQuantityDelta(int32 Delta);

	UFUNCTION()
	void OnClickedBuyButton();

	UFUNCTION()
	void OnClickedSellButton();

	UFUNCTION()
	void OnClickedCloseButton();

	UFUNCTION()
	void OnClickedAddItemQuantityButton();

	UFUNCTION()
	void OnClickedSubtractItemQuantityButton();

	// 컨트롤러 델리게이트 바인딩 핸들러
	UFUNCTION()
	void HandlePurchaseResult(const FPurchaseResponse& Response, bool bIsBuy);

	// 결과 메시지 텍스트 가져오기
	FText GetResultMessageText(const FPurchaseResponse& Response, bool bIsBuy) const;

	// 델리게이트 바인딩 관리
	void BindToPurchaseDelegate();
	void UnbindFromPurchaseDelegate();

	// 구매 델리게이트 바인딩 여부
	bool bBoundToPurchaseDelegate = false;

	// 결과 메시지 자동 숨김 타이머
	FTimerHandle ResultMessageTimerHandle;

	// 결과 메시지 표시 시간 (초)
	float ResultMessageDisplayDuration = 3.0f;

	// 결과 메시지 숨김
	void HideResultMessage() const;
};