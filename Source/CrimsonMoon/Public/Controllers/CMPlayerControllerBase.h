#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CMPlayerControllerBase.generated.h"

class UCMDataAsset_InputConfig;
class UUIManagerComponent;
class UUserWidget;

UCLASS()
class CRIMSONMOON_API ACMPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	ACMPlayerControllerBase();

	// UIManagerComponent Getter (NPC 등에서 접근용)
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUIManagerComponent* GetUIManagerComponent() const { return UIManagerComponent; }

	UFUNCTION()
	void Init();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UCMDataAsset_InputConfig> InputConfigUIDataAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUIManagerComponent> UIManagerComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> StartWidgetClass;

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	UFUNCTION()
	virtual void HandleBackAction();

	UFUNCTION()
	virtual void OnSystemMenu();

	// Pawn 빙의 이후에 UI 초기화를 위해 OnPossess 훅 추가
	virtual void OnPossess(APawn* InPawn) override;
	// 클라이언트에서 Pawn이 Replicate/할당된 이후 호출되는 훅
	virtual void OnRep_Pawn() override;
};
