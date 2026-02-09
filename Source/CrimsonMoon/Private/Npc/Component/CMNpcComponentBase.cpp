// Fill out your copyright notice in the Description page of Project Settings.


#include "Npc/Component/CMNpcComponentBase.h"

#include "Npc/Interface/CMNpcHandler.h"


UCMNpcComponentBase::UCMNpcComponentBase()
{

}

void UCMNpcComponentBase::BeginPlay()
{
	Super::BeginPlay();
	RegisterComponentToHandler();
}

void UCMNpcComponentBase::RegisterComponentToHandler()
{
	if (ICMNpcHandler* Handler = Cast<ICMNpcHandler>(GetOwner()))
	{
		if (Handler->RegisterComponent(NpcComponentType,this))
		{
			bIsRegisteredToHandler = true;
		}
	}
}

void UCMNpcComponentBase::PerformAction()
{
	//TODO: 세부 동작은 서브클래스에서 구현
	UE_LOG(LogTemp, Log, TEXT("UCMNpcComponentBase::PerformAction"));
}

void UCMNpcComponentBase::PerformStopAction()
{
	
}
