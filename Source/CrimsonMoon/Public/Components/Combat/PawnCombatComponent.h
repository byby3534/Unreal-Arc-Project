// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/PawnExtensionComponentBase.h"
#include "Enums/CMEnums.h"
#include "Structs/CMStructTypes.h"
#include "PawnCombatComponent.generated.h"

class ACMWeaponBase;
class ACMCharacterBase;


/**
 * @brief 캐릭터의 전투 기능을 관리하는 컴포넌트 클래스입니다.
 *
 * 이 컴포넌트는 공격 시 충돌 판정, 피격 처리, 액터 간 상호작용 등을 처리합니다.
 * 주로 Pawn 캐릭터와 연동되어 전투 관련 로직을 실행합니다.
 */
UCLASS()
class CRIMSONMOON_API UPawnCombatComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	#pragma region Overrides
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnHitTargetActor(AActor* HitActor);
	virtual void OnWeaponPulledFromTargetActor(AActor* InteractingActor);
	virtual void OnHitTargetActorImpl(AActor* HitActor);
	#pragma endregion

	#pragma region Weapon Management
	UFUNCTION(BlueprintCallable, Category = "Crimson Moon | Combat")
	void RegisterSpawnedWeapon(FGameplayTag InWeaponTagToResister, ACMWeaponBase* InWeaponToResister, bool bResisterAsEquippedWeapon = false);

	UFUNCTION(BlueprintCallable, Category = "Crimson Moon | Combat")
	ACMWeaponBase* GetCharacterCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentEquippedWeaponTag, Category = "Crimson Moon | Combat")
	FGameplayTag CurrentEquippedWeaponTag;

	UFUNCTION(BlueprintCallable, Category = "Crimson Moon | Combat")
	ACMWeaponBase* GetCharacterCurrentEquippedWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "Crimson Moon | Combat")
	void SetCurrentEquippedWeaponTag(const FGameplayTag& NewWeaponTag);

	UFUNCTION(BlueprintCallable, Category = "Crimson Moon | Combat")
	ACMCharacterBase* GetOwnerCharacter() const;
	#pragma endregion

	#pragma region Collision
	UFUNCTION(BlueprintCallable, Category = "Crimson Moon | Combat")
	void ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType);

	UFUNCTION(Server, Reliable)
	void ServerToggleCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType);
	#pragma endregion

protected:
	#pragma region State
	// 한 번의 공격 동안 이미 맞은 액터들을 기록하는 배열
	UPROPERTY()
	TArray<TObjectPtr<AActor>> OverlappedActors;
	#pragma endregion

	#pragma region Internal
	virtual void HandleToggleCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType);
	#pragma endregion

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Crimson Moon | Combat", meta = (AllowPrivateAccess = "true"))
	FName TargetMeshTagName;

private:
	#pragma region Weapon Data
	UPROPERTY(ReplicatedUsing = OnRep_CharacterCarriedWeaponList)
	TArray<FReplicatedWeaponEntry> CharacterCarriedWeaponList;

	TWeakObjectPtr<USkeletalMeshComponent> CachedWeaponMesh;
	#pragma endregion

	#pragma region Replication
	UFUNCTION()
	void OnRep_CharacterCarriedWeaponList();

	UFUNCTION()
	void OnRep_CurrentEquippedWeaponTag(const FGameplayTag& OldWeaponTag);

	void HandleClientSideEquipEffects(const FGameplayTag& NewWeaponTag, const FGameplayTag& OldWeaponTag);
	#pragma endregion

	#pragma region Particle Preloading
	// 스킬 파티클 프라이밍 (첫 사용 시 로딩으로 인한 프리징 방지)
	void PreloadSkillParticles(const class UCMDataAsset_WeaponData* WeaponData);

	// TODO: 별도의 컴포넌트로 수정 예정
	// 게임 시작 시 모든 무기의 파티클을 비동기로 로드
	void StartAsyncParticlePreloading();
	void LoadNextParticleAsync();

	// 비동기 파티클 로딩을 위한 타이머
	FTimerHandle ParticlePreloadTimerHandle;

	TArray<TSoftObjectPtr<class UNiagaraSystem>> PendingParticlesToLoad;

	int32 CurrentParticleLoadIndex = 0;
	#pragma endregion
};