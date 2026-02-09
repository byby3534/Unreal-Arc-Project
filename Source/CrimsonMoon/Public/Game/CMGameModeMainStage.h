// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameplayTagContainer.h"
#include "CMGameModeMainStage.generated.h"

class APlayerStart;

enum class ETeamType : uint8;
struct FGenericTeamId;
/**
 * 
 */
UCLASS()
class CRIMSONMOON_API ACMGameModeMainStage : public AGameMode
{
	GENERATED_BODY()

public:
	ACMGameModeMainStage();
	
protected:
	virtual void BeginPlay() override;

	// 플레이어 입장/퇴장 시 GameState의 플레이어 수를 관리
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// 월드에 있는 PlayerStart들을 캐싱하는 함수
	void CacheAllPlayerStarts();

public:
	// 서버에서만 호출: 컨트롤러와 선택된 캐릭터 태그 정보를 받아 Pawn 스폰 및 빙의를 수행
	void SpawnPlayerForController(APlayerController* NewPlayer, const FGameplayTag& SelectedCharacterTag);

	// 게임 종료 시 특정 맵으로 돌려보내는 함수 (ServerTravel 사용)
	UFUNCTION(BlueprintCallable, Category = "Game|Flow")
	void ReturnToExitMap();

private:
	// 선택된 캐릭터 태그에 따라 매칭되는 플레이어 Pawn 클래스들
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<APawn> DefaultPlayerPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<APawn> BladePlayerPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<APawn> ArcanistPlayerPawnClass;

	// 캐싱된 PlayerStart 목록
	UPROPERTY()
	TArray<TObjectPtr<APlayerStart>> CachedPlayerStarts;

	// 다음에 사용할 PlayerStart 인덱스
	int32 NextPlayerStartIndex = 0;

	// 게임 종료 시 이동할 맵 URL (에디터에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game|Flow", meta = (AllowPrivateAccess = "true"))
	FString ExitMapTravelURL;
};