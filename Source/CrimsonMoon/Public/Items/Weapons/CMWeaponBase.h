// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "CMWeaponBase.generated.h"

class UBoxComponent;
DECLARE_DELEGATE_OneParam(FonTargetInteractedDelegate, AActor*)

UCLASS()
class CRIMSONMOON_API ACMWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ACMWeaponBase();

	virtual void BeginPlay() override;

	FonTargetInteractedDelegate OnWeaponHitTarget;
	FonTargetInteractedDelegate OnWeaponPulledFromTarget;

	// 저장된 이펙트 핸들을 저장하는 함수
	virtual void AddGrantedGameplayEffect(FActiveGameplayEffectHandle Handle);

	// 저장된 핸들 목록을 반환하고 비우는 함수 (해제용)
	virtual TArray<FActiveGameplayEffectHandle> RemoveGrantedGameplayEffects();

	// 스폰 직후 호출해서 무기 숨김
	void HideWeapon();

	// 장착 이벤트 시 호출해서 무기 보임
	void ShowWeapon();

	bool GetHideUntilEquipped() const { return bHideUntilEquipped; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
	UBoxComponent* WeaponCollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons|Combat", meta = (AllowPrivateAccess = "true"))
	bool bIgnoreFriendly = true;

	// true인 경우, 무기가 숨겨지고 장착 이벤트 때 보여짐
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons|Visibility")
	bool bHideUntilEquipped = false;

	UFUNCTION()
	virtual void OnCollisionBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	UFUNCTION()
	virtual void OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 이 무기가 장착되면서 적용한 이펙트들의 핸들 목록
	TArray<FActiveGameplayEffectHandle> GrantedEffectHandles;

public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const { return WeaponCollisionBox; }
};