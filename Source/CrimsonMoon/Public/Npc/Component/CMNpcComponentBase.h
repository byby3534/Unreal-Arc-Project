// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Npc/CMNpcBase.h"
#include "CMNpcComponentBase.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMSONMOON_API UCMNpcComponentBase : public UActorComponent
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	UCMNpcComponentBase();

protected:
	virtual void BeginPlay() override;

	/* Custom Methods */
public:
	virtual void RegisterComponentToHandler();
	virtual void PerformAction();
	virtual void PerformStopAction();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC Component")
	ECMNpcComponentType NpcComponentType;
	
private:
	bool bIsRegisteredToHandler = false;
};