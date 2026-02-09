// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CMFunctionLibrary.generated.h"

enum class ECMValidType : uint8;
class UPawnCombatComponent;
class UCMAbilitySystemComponent;
struct FGameplayTag;

/**
 * 정적 헬퍼 메서드를 제공하는 유틸리티 함수 라이브러리입니다.
 *
 * 이 라이브러리는 UBlueprintFunctionLibrary를 상속받아,
 * 블루프린트 및 C++ 전역에서 호출 가능한 정적 유틸리티 커스텀 함수를 제공합니다.
 */
UCLASS()
class CRIMSONMOON_API UCMFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* 액터에서 커스텀 어빌리티 시스템 컴포넌트를 가져옴 */
	static UCMAbilitySystemComponent* NativeAbilitySystemComponentFromActor(AActor* InActor);

	/* 액터에 특정 GameplayTag가 없을 경우 추가 */
	UFUNCTION(BlueprintCallable, Category = "Crimson Moon|FunctionLibrary")
	static void AddGameplayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd);

	/* 액터가 지정된 GameplayTag를 가지고 있는지 확인 */
	static bool NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck);

	/* 두 Pawn이 서로 적대 관계인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Crimson Moon|FunctionLibrary")
	static bool IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn);

	/* 액터에서 PawnCombatComponent를 가져옴 */
	UFUNCTION(BlueprintCallable, Category = "Crimson Moon|FunctionLibrary", meta = (DisplayName = "Get Pawn Combat Component From Actor", ExpandEnumAsExecs = "OutValidType"))
	static UPawnCombatComponent* BP_GetPawnCombatComponentFromActor(AActor* InActor, ECMValidType& OutValidType);

	/* 내부적으로 PawnCombatComponent를 직접 검색 */
	static UPawnCombatComponent* NativeGetPawnCombatComponentFromActor(AActor* InActor);

	/* 캐릭터가 맞은 위치 별 태그 반환 */
	UFUNCTION(BlueprintPure, Category = "Crimson Moon|FunctionLibrary")
	static FGameplayTag ComputeHitReactDirectionTag(const AActor* InAttacker, const AActor* InVictim, float& OutAngleDifference);

	/**
	 * Actor에서 특정 태그를 가진 SkeletalMeshComponent를 찾아 반환합니다.
	 * @param InActor 검색할 액터
	 * @param ComponentTag 찾을 컴포넌트 태그
	 * @param bUseFallbackMesh 못 찾으면 캐릭터의 기본 메시 반환 여부
	 * @return 찾은 메시 (없으면 nullptr 또는 기본 메시)
	 */
	UFUNCTION(BlueprintCallable, Category = "Crimson Moon|FunctionLibrary")
	static USkeletalMeshComponent* FindSkeletalMeshByTag(
		const AActor* InActor,
		FName ComponentTag,
		bool bUseFallbackMesh = true
		);

	/**
	 * 방어자가 공격자를 향해 블록 가능한 각도 범위 내에 있는지 확인합니다.
	 * @param InAttacker 공격자
	 * @param InDefender 방어자
	 * @param AngleThreshold 블록 허용 각도 (도 단위, 기본값 60도)
	 * @return 유효한 블록이면 true, 아니면 false
	 */
	UFUNCTION(BlueprintPure, Category = "Crimson Moon|FunctionLibrary")
	static bool IsValidBlock(const AActor* InAttacker, const AActor* InDefender, const float AngleThreshold = 60.0f);

private:
	/* 주어진 각도 차이를 바탕으로 히트 반응 태그를 결정 */
	static FGameplayTag DetermineHitReactionTag(const float& OutAngleDifference);

};