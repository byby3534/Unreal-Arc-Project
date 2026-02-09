#pragma once

#include "CoreMinimal.h"
#include "CMGameStateBase.h"
#include "CMGameStateMainStageV2.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSpawn, ACMEnemyCharacterBase*, NewBoss);

class ACMEnemyCharacterBase;
/**
 * 
 */
UCLASS()
class CRIMSONMOON_API ACMGameStateMainStageV2 : public ACMGameStateBase
{
	GENERATED_BODY()

	/* Engine Methods */
public:
	ACMGameStateMainStageV2();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	/* Custom Methods */
public:
	void IncrementConnectedPlayerCount();
	void DecrementConnectedPlayerCount();
	void IncrementDeathCount();
	void DecrementDeathCount();

	// 게임 승리를 처리하는 함수 (서버에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Game|Result")
	void HandleGameVictory();

	// 모든 클라이언트에게 승리 UI를 띄우도록 지시하는 멀티캐스트 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowVictoryUI();

	// 모든 클라이언트에게 패배 UI를 띄우도록 지시하는 멀티캐스트 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowDefeatUI();

private:
	// 현재 카운트 상태를 기반으로 모든 플레이어가 사망했는지 검사하고,
	// 조건이 만족되면 딜레이 후 로비로 이동시키는 타이머를 설정하는 헬퍼 함수
	void CheckAllPlayersDeadAndScheduleReturn();

protected:
	int32 CurrentPlayerCount = 0;
	int32 DeathCount = 0;

	// 로비로 돌아가기 전 대기할 시간(초). 에디터에서 수정 가능
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game|Flow")
	float ReturnToLobbyDelay = 7.0f;

	#pragma region Boss
public:
	FOnBossSpawn OnBossSpawned;

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void RegisterBoss(ACMEnemyCharacterBase* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	FORCEINLINE ACMEnemyCharacterBase* GetCurrentBoss() const { return CurrentBoss; }

	UFUNCTION()
	void OnRep_CurrentBoss();

private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentBoss, Transient)
	TObjectPtr<ACMEnemyCharacterBase> CurrentBoss;
#pragma endregion
};
