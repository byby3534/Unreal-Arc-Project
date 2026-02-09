// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CMNpcComponentBase.h"
#include "CMNpcDialogueComponent.generated.h"

// 이 컴포넌트는 이제 별도 로직 없이 타입만 유지 (필요시 나중에 확장)
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRIMSONMOON_API UCMNpcDialogueComponent : public UCMNpcComponentBase
{
	GENERATED_BODY()

public:
	UCMNpcDialogueComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 현재는 대화 로직을 ACMNpcBase로 옮겼으므로 비워둠
};