// Fill out your copyright notice in the Description page of Project Settings.


#include "CMFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CMGameplayTags.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayAbilitySystem/CMAbilitySystemComponent.h"
#include "Interfaces/PawnCombatInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "CrimsonMoon/DebugHelper.h"

UCMAbilitySystemComponent* UCMFunctionLibrary::NativeAbilitySystemComponentFromActor(AActor* InActor)
{
	check(InActor);

	return CastChecked<UCMAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
}

void UCMFunctionLibrary::AddGameplayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
	UCMAbilitySystemComponent* ASC = NativeAbilitySystemComponentFromActor(InActor);
	if (!ASC->HasMatchingGameplayTag(TagToAdd))
	{
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

bool UCMFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck)
{
	UCMAbilitySystemComponent* ASC = NativeAbilitySystemComponentFromActor(InActor);

	return ASC->HasMatchingGameplayTag(TagToCheck);
}

bool UCMFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	check(QueryPawn && TargetPawn);

	const IGenericTeamAgentInterface* GenericTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
	const IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());

	if (GenericTeamAgent && TargetTeamAgent)
	{
		// 팀 ID가 다르면 적대 관계로 간주하여 true 반환
		return GenericTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
	}

	// 팀 인터페이스를 구현하지 않은 경우 적대 관계로 간주하지 않음
	return false;
}

UPawnCombatComponent* UCMFunctionLibrary::BP_GetPawnCombatComponentFromActor(AActor* InActor, ECMValidType& OutValidType)
{
	UPawnCombatComponent* CombatComponent = NativeGetPawnCombatComponentFromActor(InActor);
	OutValidType = CombatComponent ? ECMValidType::Valid : ECMValidType::Invalid;
	return CombatComponent;
}

UPawnCombatComponent* UCMFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* InActor)
{
	check(InActor);

	if (const IPawnCombatInterface* PawnCombatInterface = Cast<IPawnCombatInterface>(InActor))
	{
		return PawnCombatInterface->GetPawnCombatComponent();
	}

	return nullptr;
}

FGameplayTag UCMFunctionLibrary::ComputeHitReactDirectionTag(const AActor* InAttacker, const AActor* InVictim, float& OutAngleDifference)
{
	check(InAttacker && InVictim);

	const FVector VictimForward = InVictim->GetActorForwardVector();
	const FVector VictimToAttackerNormalized = (InAttacker->GetActorLocation() - InVictim->GetActorLocation()).GetSafeNormal();

	// 두 벡터의 내적 결과 (코사인 값)를 구함
	const float DotResult = FVector::DotProduct(VictimForward, VictimToAttackerNormalized);
	const FVector CrossResult = FVector::CrossProduct(VictimForward, VictimToAttackerNormalized);

	OutAngleDifference = UKismetMathLibrary::DegAcos(DotResult);

	// 외적 결과의 Z 값이 음수이면 오른쪽에서 공격 -> 각도 부호를 음수로 바꿈
	if (CrossResult.Z < 0.f)
	{
		OutAngleDifference *= -1.f;
	}
	return DetermineHitReactionTag(OutAngleDifference);
}

FGameplayTag UCMFunctionLibrary::DetermineHitReactionTag(const float& OutAngleDifference)
{
	// -45 ~ 45도 = 정면
	if (OutAngleDifference >= -45.f && OutAngleDifference <= 45.f)
	{
		return CMGameplayTags::Shared_Status_HitReact_Front;
	}
	// -135 ~ -45도 = 왼쪽
	if (OutAngleDifference < -45.f && OutAngleDifference >= -135.f)
	{
		return CMGameplayTags::Shared_Status_HitReact_Left;
	}
	// - 135보다 작거나 135보다 크면 = 오른쪽
	if (OutAngleDifference < -135.f || OutAngleDifference > 135.f)
	{
		return CMGameplayTags::Shared_Status_HitReact_Back;
	}
	// 45 ~ 135도 = 뒤
	if (OutAngleDifference > 45.f && OutAngleDifference <= 135.f)
	{
		return CMGameplayTags::Shared_Status_HitReact_Right;
	}
	return CMGameplayTags::Shared_Status_HitReact_Front;
}

USkeletalMeshComponent* UCMFunctionLibrary::FindSkeletalMeshByTag(const AActor* InActor, FName ComponentTag, bool bUseFallbackMesh)
{
	if (!InActor)
	{
		return nullptr;
	}

	// Actor의 모든 SkeletalMeshComponent 가져오기
	TArray<USkeletalMeshComponent*> MeshComponents;
	InActor->GetComponents<USkeletalMeshComponent>(MeshComponents);

	// ComponentTag를 가진 메시 찾기
	for (USkeletalMeshComponent* Mesh : MeshComponents)
	{
		if (Mesh && Mesh->ComponentHasTag(ComponentTag))
		{
			return Mesh;
		}
	}

	// 못 찾았을 때 처리
	if (bUseFallbackMesh)
	{
		// ACharacter인 경우 기본 메시 반환
		if (const ACharacter* Character = Cast<ACharacter>(InActor))
		{
			return Character->GetMesh();
		}
	}

	return nullptr;
}

bool UCMFunctionLibrary::IsValidBlock(const AActor* InAttacker, const AActor* InDefender, const float AngleThreshold)
{
	if (!InAttacker || !InDefender)
	{
		return false;
	}

	// 각도를 코사인 값으로 변환 (내적 비교를 위해)
	const float AllowDotResult = FMath::Cos(FMath::DegreesToRadians(AngleThreshold));

	// 방어자의 정면 방향
	const FVector DefenderForward = InDefender->GetActorForwardVector();

	// 방어자 -> 공격자 방향 벡터 계산
	const FVector DirectionVector = InAttacker->GetActorLocation() - InDefender->GetActorLocation();

	// 위치가 거의 겹칠 정도로 가까우면 블록 성공 및 패링 처리
	if (DirectionVector.IsNearlyZero())
	{
		return true;
	}

	// 방어자 -> 공격자 방향 정규화
	const FVector DefenderToAttacker = DirectionVector.GetSafeNormal();

	// 방어자가 공격자를 제대로 향하고 있는지 확인
	const float DotResult = FVector::DotProduct(DefenderForward, DefenderToAttacker);

	// AngleThreshold 각도 이내에서 블록 성공
	return DotResult > AllowDotResult;
}