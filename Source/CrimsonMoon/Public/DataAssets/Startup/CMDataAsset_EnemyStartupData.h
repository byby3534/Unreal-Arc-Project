// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Startup/CMDataAsset_StartupDataBase.h"
#include "CMDataAsset_EnemyStartupData.generated.h"

/**
 * @class UCMDataAsset_EnemyStartupData
 * @brief 적 캐릭터의 초기화 데이터를 정의하는 클래스입니다.
 *
 * 이 클래스는 적 캐릭터가 생성될 때 초기화에 필요한
 * 데이터와 설정을 관리합니다. 프로젝트 내의 UCMDataAsset_StartupDataBase
 * 클래스를 상속받아 확장된 기능을 제공합니다.
 *
 * UCMDataAsset_StartupDataBase에 정의된 다양한 초기화 데이터를 기반으로,
 * 적 전용 설정이나 능력을 추가적으로 정의할 수 있습니다.
 */
UCLASS()
class CRIMSONMOON_API UCMDataAsset_EnemyStartupData : public UCMDataAsset_StartupDataBase
{
	GENERATED_BODY()

public:
	virtual void GiveToAbilitySystemComponent(UCMAbilitySystemComponent* InASCToGive, int32 ApplyLevel = 1) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "StartUpData", meta = (TitleProperty = "InputTag"))
	TArray<TSubclassOf<UCMGameplayAbility>> EnemyGameplayAbility;
};