// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CMMapGenerate.generated.h"

class UCMRoomDataDefinition;
// This class does not need to be modified.
UINTERFACE()
class UCMMapGenerate : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CRIMSONMOON_API ICMMapGenerate
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void GenerateMap(int32 InSeed, UCMRoomDataDefinition* InMapData) = 0;

};