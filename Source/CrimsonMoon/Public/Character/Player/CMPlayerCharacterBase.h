// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CMCharacterBase.h"
#include "CMPlayerCharacterBase.generated.h"

struct FInputActionValue;
struct FGameplayTag;
class UPlayerCombatComponent;
class UCMLineTraceComponent;
class UCameraComponent;
class USpringArmComponent;
class UCMDataAsset_InputConfig;
class UGameplayCameraComponent;
/**
 *
 */
UCLASS()
class CRIMSONMOON_API ACMPlayerCharacterBase : public ACMCharacterBase
{
	GENERATED_BODY()

public:
	ACMPlayerCharacterBase();

	#pragma region Overrides
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPlayerCombatComponent* GetPawnCombatComponent() const override;
	#pragma endregion

	#pragma region Getter
	FORCEINLINE float GetTargetLockMaxWalkSpeed() const { return TargetLockMaxWalkSpeed; }
	FORCEINLINE float GetCachedDefaultMaxWalkSpeed() const { return CachedDefaultMaxWalkSpeed; }
	FORCEINLINE float GetTargetLockRotationInterpSpeed() const { return TargetLockRotationInterpSpeed; }
	FORCEINLINE FVector GetCachedRollInputDirection() const { return CachedRollInputDirection; }
	FORCEINLINE UStaticMeshComponent* GetHandHeldItemMesh() const { return HandHeldItemMesh; }
	FORCEINLINE UCMDataAsset_InputConfig* GetInputConfigDataAsset() const { return InputConfigDataAsset; }
	#pragma endregion

	/** Roll 입력 시 Movement Input 방향 캐싱 (서버에서 사용) */
	void CacheRollInputDirection();

	/** 서버에 Roll 입력 방향 즉시 전송 (RPC) */
	UFUNCTION(Server, Reliable)
	void ServerCacheRollInputDirection(FVector_NetQuantize100 InputDirection);

	/** 서버에 퀵슬롯 변경 요청 (RPC) */
	UFUNCTION(Server, Reliable)
	void Server_SetActiveSlot(int32 SlotIndex);

	/** 서버에 아이템 순환 요청 (RPC) */
	UFUNCTION(Server, Reliable)
	void Server_CycleActiveItem(int32 Direction);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData | DataAsset", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCMDataAsset_InputConfig> InputConfigDataAsset;

	#pragma region Lock On
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crimson Moon | LockOn")
	float TargetLockMaxWalkSpeed = 150.f;

	UPROPERTY(BlueprintReadOnly, Category = "Crimson Moon | LockOn")
	float CachedDefaultMaxWalkSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Crimson Moon | LockOn")
	float TargetLockRotationInterpSpeed = 10.f;
	#pragma endregion

	#pragma region Character Class

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Class", meta = (Categories = "Character.Class"))
	FGameplayTag CharacterClassTag;

	#pragma endregion

	// 퀵슬롯 연속 스왑 간격
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float QuickSlotSwapInterval = 1.0f;
	
private:
	// BeginPlay 한 프레임 뒤에 GameplayCamera를 셋업하는 헬퍼 함수
	void SetupGameplayCamera_Helper();

	/** Roll 입력 시의 Movement Input 방향 (복제됨) */
	UPROPERTY(Replicated)
	FVector_NetQuantize100 CachedRollInputDirection;

	#pragma region Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera | SpringArm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> ViewCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerCombatComponent> PlayerCombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCMLineTraceComponent> LineTraceComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HandHeldItemMesh;
	#pragma endregion

	#pragma region  Input Callback
	void Input_AbilityInputPressed(const FGameplayTag InInputTag);
	void Input_AbilityInputReleased(const FGameplayTag InInputTag);
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_QuickSlot_Potion_Started(const FInputActionValue& Value);
	void Input_QuickSlot_Potion_Tap(const FInputActionValue& InputActionValue);
	void Input_QuickSlot_Potion_Hold(const FInputActionValue& InputActionValue);
	void Input_QuickSlot_Utility_Started(const FInputActionValue& Value);
	void Input_QuickSlot_Utility_Tap(const FInputActionValue& InputActionValue);
	void Input_QuickSlot_Utility_Hold(const FInputActionValue& InputActionValue);
	#pragma endregion

	#pragma region Input
public:
	void SetGameplayInputEnabled(bool bEnabled);

protected:
	bool bIsGameplayInputEnabled = true;
	
	UPROPERTY()
	TObjectPtr<UInputMappingContext> CurrentWeaponMappingContext;
	#pragma endregion

	// 입력 처리 함수 (Started, Canceled, Triggered 모두 처리)
	double LastPotionSwapTime = 0.0;
	double LastUtilitySwapTime = 0.0;
};