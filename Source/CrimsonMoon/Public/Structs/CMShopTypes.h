// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CMShopTypes.generated.h"

/**
 * 상점 구매/판매 결과 코드
 */
UENUM(BlueprintType)
enum class EPurchaseResult : uint8
{
	Success				UMETA(DisplayName = "Success"),
	InsufficientFunds	UMETA(DisplayName = "Insufficient Funds"),
	InvalidProduct		UMETA(DisplayName = "Invalid Product"),
	InvalidQuantity		UMETA(DisplayName = "Invalid Quantity"),
	InventoryFull		UMETA(DisplayName = "Inventory Full"),
	ItemNotOwned		UMETA(DisplayName = "Item Not Owned"),
	ServerError			UMETA(DisplayName = "Server Error  "),
};

/**
 * 상점 거래 응답 구조체
 */
USTRUCT(BlueprintType)
struct FPurchaseResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	EPurchaseResult Result = EPurchaseResult::ServerError;

	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	FName ItemID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	int32 TotalPrice = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Shop")
	int32 RemainingCurrency = 0;

	FPurchaseResponse() = default;

	FPurchaseResponse(EPurchaseResult InResult, FName InItemID = NAME_None, int32 InQuantity = 0, int32 InTotalPrice = 0, int32 InRemainingCurrency = 0)
		: Result(InResult)
		, ItemID(InItemID)
		, Quantity(InQuantity)
		, TotalPrice(InTotalPrice)
		, RemainingCurrency(InRemainingCurrency)
	{
	}

	bool IsSuccess() const { return Result == EPurchaseResult::Success; }
};
