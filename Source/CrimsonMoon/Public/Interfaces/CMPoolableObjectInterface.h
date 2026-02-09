// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CMPoolableObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCMPoolableObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CRIMSONMOON_API ICMPoolableObjectInterface
{
	GENERATED_BODY()

public:
	// 오브젝트 활성화
	UFUNCTION(BlueprintNativeEvent, Category = "CM|Pooling")
	void PoolActivate();

	// 풀로 반납되어 비활성화될 때 호출 (상태 리셋)
	UFUNCTION(BlueprintNativeEvent, Category = "CM|Pooling")
	void PoolReturn();

	// 풀에서 사용 가능한지 확인
	UFUNCTION(BlueprintNativeEvent, Category = "CM|Pooling")
	bool IsAvailable() const;

	// 해당 오브젝트를 풀로 반납하는 명령
	UFUNCTION(BlueprintNativeEvent, Category = "CM|Pooling")
	void ReleaseToPool();
};
