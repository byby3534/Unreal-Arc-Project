// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "CMPickUpComponent.generated.h"

class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;

// UI에게 보낼 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, AActor*, NewTarget);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CRIMSONMOON_API UCMPickUpComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCMPickUpComponent();

	// 게임 시작 시 스스로 초기화
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 프레임 가장 가까운 아이템 찾기
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 현재 가장 가까운 아이템 반환
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractable() const { return CurrentInteractableActor.Get(); }

	// UI 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableFound OnInteractableFound;

protected:
	
	// [입력 설정]
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> PickupMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PickupAction;

	// [GAS 설정] 이 태그를 가진 어빌리티(GA_Interact)를 실행
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag InteractAbilityTag;

	// ASC 캐싱
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	void OnPickupInput(const FInputActionValue& Value);
	
	// 오버랩된 액터 목록
	TArray<TWeakObjectPtr<AActor>> OverlappingActors;

	// 현재 선택된(가장 가까운) 타겟
	UPROPERTY(ReplicatedUsing = OnRep_CurrentInteractableActor)
	TWeakObjectPtr<AActor> CurrentInteractableActor;

	UFUNCTION()
	void OnRep_CurrentInteractableActor();

	UFUNCTION()
	void OnOwnerOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnOwnerOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

	// 가장 가까운 녀석 찾기
	void FindClosestInteractable();

private:
	// 입력 시스템 초기화 시도 함수
	void InitializeInputSystem();

	// 거리 계산이 필요한지 여부 체크 플래그
	bool bNeedUpdate = false;

	// 재시도용 타이머 핸들
	FTimerHandle RetryInitTimerHandle;
};
