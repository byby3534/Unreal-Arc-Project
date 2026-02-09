// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Structs/CMStructTypes.h"
#include "CMInteractableInterface.generated.h"

class UDataAsset;
struct FInteractionUIData;

/**
 * 
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UCMInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 실제 기능을 정의하는 IInterface 클래스
 */
class CRIMSONMOON_API ICMInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* Interactor);

	// UI에 표시할 데이터를 반환
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	FInteractionUIData GetInteractableData();
};
