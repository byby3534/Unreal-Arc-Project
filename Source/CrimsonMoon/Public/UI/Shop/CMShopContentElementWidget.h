// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Npc/CMNpcBase.h"
#include "CMShopContentElementWidget.generated.h"

class UCMShopWidget;
class UImage;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopItemActionRequested, UCMShopContentElementWidget*, Widget);

UCLASS()
class CRIMSONMOON_API UCMShopContentElementWidget : public UUserWidget
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	
	
protected:
	virtual void NativeOnInitialized() override;
	
public:
	// 아이템 정보 표시 세팅 (구매/판매 가격 분리)
	UFUNCTION(BlueprintCallable, Category = "Shop|Element")
	void SetItemDisplayData(const FCMShopItemContent& InItem, UCMShopWidget* InShopInstance);

	FCMShopItemContent GetItemContent() { return ItemData; }

protected:
	// UMG 디자이너에서 아래 이름으로 위젯 바인딩
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText;

	// 구매/판매 가격을 각각 표시할 TextBlock
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BuyPriceText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SellPriceText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SelectButton;

private:
	FCMShopItemContent ItemData;

	UPROPERTY()
	TObjectPtr<UCMShopWidget> OwningShopInstance;
	
	UFUNCTION()
	void HandleContentClicked();
};
