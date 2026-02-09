#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CMBossBattleTrigger.generated.h"

class UBoxComponent;
class ACMEnemyCharacterBase;

UCLASS()
class CRIMSONMOON_API ACMBossBattleTrigger : public AActor
{
	GENERATED_BODY()
	
public:
	ACMBossBattleTrigger();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<ACMEnemyCharacterBase> BossReference;

	bool bHasTriggered;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
