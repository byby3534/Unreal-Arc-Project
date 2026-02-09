// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Components/UI/UPawnUIComponent.h"
#include "PawnUIInterface.generated.h"

UINTERFACE()
class UPawnUIInterface : public UInterface
{
	GENERATED_BODY()
};

class CRIMSONMOON_API IPawnUIInterface
{
	GENERATED_BODY()

public:
	virtual UPawnUIComponent* GetPawnUIComponent() const = 0;
};