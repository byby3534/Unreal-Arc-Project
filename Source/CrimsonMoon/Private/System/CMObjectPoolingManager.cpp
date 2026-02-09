// Fill out your copyright notice in the Description page of Project Settings.

#include "System/CMObjectPoolingManager.h"
#include "Interfaces/CMPoolableObjectInterface.h"

AActor* UCMObjectPoolingManager::GetObject(const TSubclassOf<AActor>& ActorClass)
{
	// 클라이언트에서는 실행 안 함
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return nullptr;
	}
	
	check(ActorClass && ActorClass->ImplementsInterface(UCMPoolableObjectInterface::StaticClass()));
	
	FCMObjectPool& Pool = ObjectPools.FindOrAdd(ActorClass);

	for (AActor* Actor : Pool.Objects)
	{
		if (Actor && ICMPoolableObjectInterface::Execute_IsAvailable(Actor))
		{
			return Actor;
		}
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Actor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity, SpawnParameters);

	if (Actor)
	{
		Pool.Objects.Add(Actor);
		ICMPoolableObjectInterface::Execute_PoolReturn(Actor);
		return Actor;
	}

	return nullptr;
}

void UCMObjectPoolingManager::ActivateObject(AActor* Actor)
{
	if (Actor && Actor->Implements<UCMPoolableObjectInterface>())
	{
		ICMPoolableObjectInterface::Execute_PoolActivate(Actor);
	}
}

void UCMObjectPoolingManager::ReturnObject(AActor* Actor)
{
	if (Actor && Actor->Implements<UCMPoolableObjectInterface>())
	{
		ICMPoolableObjectInterface::Execute_PoolReturn(Actor);
	}
}

void UCMObjectPoolingManager::PreparePool(const TSubclassOf<AActor>& ActorClass, int32 Count)
{
	check(ActorClass && ActorClass->ImplementsInterface(UCMPoolableObjectInterface::StaticClass()));

	FCMObjectPool& Pool = ObjectPools.FindOrAdd((ActorClass));

	for (int32 i = 0; i < Count; ++i)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* Actor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity, SpawnParameters);

		if (Actor)
		{
			ICMPoolableObjectInterface::Execute_PoolReturn(Actor);
			Pool.Objects.Add(Actor);
		}
	}
}