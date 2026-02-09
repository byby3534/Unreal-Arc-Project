// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CMObjectPoolingManager.generated.h"

USTRUCT()
struct FCMObjectPool
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> Objects;
};

UCLASS()
class CRIMSONMOON_API UCMObjectPoolingManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 풀에서 사용 가능한 오브젝트를 가져오거나 없으면 새로 생성
	UFUNCTION(BlueprintCallable)
	AActor* GetObject(const TSubclassOf<AActor>& ActorClass);

	// 초기화가 완료된 오브젝트 활성화
	UFUNCTION(BlueprintCallable)
	void ActivateObject(AActor* Actor);

	// 사용이 끝난 오브젝트를 비활성화하여 풀에 반납
	void ReturnObject(AActor* Actor);
    
	// 풀을 지정된 수량의 오브젝트로 미리 생성
	UFUNCTION(BlueprintCallable)
	void PreparePool(const TSubclassOf<AActor>& ActorClass, int32 Count);

private:
	/**
	 * 투사체 클래스별로 풀을 관리하기 위한 맵
	 * Key: 투사체 클래스, Value: 해당 클래스의 풀에 포함된 투사체 배열
	 */
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FCMObjectPool> ObjectPools;
};