// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CMNpcHandler.generated.h"

class UCMNpcComponentBase;
enum class ECMNpcComponentType: uint8;
// This class does not need to be modified.
UINTERFACE()
class UCMNpcHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CRIMSONMOON_API ICMNpcHandler
{
	GENERATED_BODY()

public:
	virtual void HandleActionByType(ECMNpcComponentType ComponentType) = 0;
	virtual bool RegisterComponent(ECMNpcComponentType ComponentType, UCMNpcComponentBase* NewComponent) = 0;

};