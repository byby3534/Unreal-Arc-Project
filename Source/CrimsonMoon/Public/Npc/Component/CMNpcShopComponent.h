// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Npc/Component/CMNpcComponentBase.h"
#include "CMNpcShopComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMSONMOON_API UCMNpcShopComponent : public UCMNpcComponentBase
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	UCMNpcShopComponent();

protected:
	virtual void BeginPlay() override;

	/* Custom Methods */
public:
	virtual void PerformAction() override;

	FORCEINLINE void SetShopItemContents(const TArray<FCMShopItemContent>& InItems) { ShopItemContents = InItems; }
	FORCEINLINE void GetShopItemContents(TArray<FCMShopItemContent>& OutItems) const { OutItems = ShopItemContents; }
	
private:
	UPROPERTY()
	TArray<FCMShopItemContent> ShopItemContents;
};