// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "CMRoomBoundsBox.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMSONMOON_API UCMRoomBoundsBox : public UBoxComponent
{
	GENERATED_BODY()

public:
	UCMRoomBoundsBox();

protected:
	virtual void BeginPlay() override;

public:

};