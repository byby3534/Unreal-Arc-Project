// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CMItemInstance.generated.h"

class UCMDataAsset_ItemBase;

/**
 * 아이템 인스턴스
 */
UCLASS()
class CRIMSONMOON_API UCMItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UCMItemInstance();

	// uObject가 네트워크 복제를 지원하도록 설정
	virtual bool IsSupportedForNetworking() const override { return true; }
    
	// 변수 복제 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
	// 데이터 에셋
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UCMDataAsset_ItemBase> ItemData;

	// 개수
	UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity;

	// 수량이 변경될 때 클라이언트에서 호출
	UFUNCTION()
	void OnRep_Quantity();
};
