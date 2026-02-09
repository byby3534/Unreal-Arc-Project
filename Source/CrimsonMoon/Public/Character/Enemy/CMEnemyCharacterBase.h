// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CMCharacterBase.h"
#include "AI/CMAITypes.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "CMEnemyCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyStateChangedDelegate, ECMEnemyState, NewState);

class UWidgetComponent;
class UEnemyCombatComponent;
class UCMDataAsset_MonsterLoot;

/**
 *
 */
UCLASS()
class CRIMSONMOON_API ACMEnemyCharacterBase : public ACMCharacterBase
{
	GENERATED_BODY()

public:
	ACMEnemyCharacterBase();

	// 액터의 리플리케이트 프로퍼티
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 에디터에서 프로퍼티 변경 시 호출 (본 이름 변경 시 콜리전 박스 재부착)
	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAIState(ECMEnemyState NewState);

	UFUNCTION(BlueprintCallable, Category = "AI")
	FORCEINLINE ECMEnemyState GetAIState() const { return AIState; }

	virtual UEnemyCombatComponent* GetPawnCombatComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnemyCombatComponent> EnemyCombatComponent;

	// 스테이트 트리가 직접 바인딩할 메인 타겟 블랙보드 대신 이 프로퍼티를 사용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "AI|StateTree")
	TObjectPtr<AActor> StateTreeTargetActor;

	// 이 몬스터가 가진 전리품 데이터 (에디터에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UCMDataAsset_MonsterLoot> LootData;

	#pragma region Currency Reward

	/**
	 * 이 몬스터를 처치 시 지급되는 재화량 (Soul/Rune)
	 * 에디터에서 몬스터별로 설정 가능합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency", meta = (ClampMin = "0"))
	int32 RewardCurrency = 100;

	/**
	 * 처치 보상 지급 요청
	 * 서버에서만 실행되며, 킬러에게 RewardCurrency를 지급
	 * 중복 호출 방지 처리가 되어 있음
	 *
	 * @param Killer 처치자 (PlayerCharacter 또는 해당 ASC를 가진 액터)
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void GrantKillReward(AActor* Killer);

	/**
	 * 보상이 이미 지급되었는지 확인.
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool HasRewardBeenGiven() const { return bRewardGiven; }

	#pragma endregion

	UFUNCTION(BlueprintPure, Category = "UI")
	FText GetEnemyName() const;

	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnEnemyStateChangedDelegate OnEnemyStateChanged;

	FORCEINLINE TArray<TObjectPtr<AActor>> GetPatrolPoints() const { return PatrolPoints; }
	FORCEINLINE TObjectPtr<UBoxComponent> GetRightHandCollisionBox() const { return RightHandCollisionBox; }
	FORCEINLINE TObjectPtr<UBoxComponent> GetLeftHandCollisionBox() const { return LeftHandCollisionBox; }

protected:
	/**
	 * 보상 지급 완료 플래그
	 * 네트워크/애니메이션/클라이언트에서 사망 이벤트가 중복 호출되어도
	 * 보상이 1회만 지급되도록 방지
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Currency")
	bool bRewardGiven = false;

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION()
	virtual void OnRep_AIState();

	// GAS 태그가 변경되었을 때 호출될 콜백 함수
	virtual void OnStateTagChanged(const FGameplayTag Tag, int32 NewCount);

	/**
	 * 사망 이벤트 처리 콜백
	 * Shared.Event.Death GameplayEvent 수신 시 호출
	 * 서버에서만 보상 지급 로직이 실행
	 *
	 * @param EventTag 이벤트 태그
	 * @param Payload 이벤트 데이터 (Instigator = 킬러)
	 */
	void OnDeathEventReceived(FGameplayTag EventTag, const struct FGameplayEventData* Payload);

	/** 사망 이벤트 델리게이트 핸들 */
	FDelegateHandle DeathEventDelegateHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText EnemyName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AIState, Category = "AI")
	ECMEnemyState AIState;

	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	TArray<TObjectPtr<AActor>> PatrolPoints;

	#pragma region Combat Collision
	// 맨손 공격 콜리전 Overlap 콜백
	UFUNCTION()
	void OnHandCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	// 맨손 공격용 콜리전 박스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> RightHandCollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> LeftHandCollisionBox;

	// 콜리전 박스를 부착할 본 이름 (에디터에서 변경 가능)
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = true))
	FName RightHandCollisionBoxAttachBoneName = FName("RightHandSocket");

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = true))
	FName LeftHandCollisionBoxAttachBoneName = FName("LeftHandSocket");
	#pragma endregion

	#pragma region Widget
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;
	#pragma endregion
};