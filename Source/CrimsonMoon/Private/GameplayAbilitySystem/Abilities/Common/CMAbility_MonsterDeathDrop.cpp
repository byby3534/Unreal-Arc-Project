// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilitySystem/Abilities/Common/CMAbility_MonsterDeathDrop.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "DataAssets/CMDataAsset_MonsterLoot.h"
#include "DataAssets/CMDataAsset_ItemBase.h"
#include "Items/Pickups/CMPickupActor.h"
#include "Kismet/GameplayStatics.h"
#include "CMGameplayTags.h"

UCMAbility_MonsterDeathDrop::UCMAbility_MonsterDeathDrop()
{
    // 아이템 스폰은 반드시 서버에서만 일어납니다.
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 사망 이벤트 태그에 반응하도록 설정
    FAbilityTriggerData Trigger;
    Trigger.TriggerTag = CMGameplayTags::Shared_Event_Death;
    Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trigger);
}

void UCMAbility_MonsterDeathDrop::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UE_LOG(LogTemp, Warning, TEXT("[MonsterDeathDrop] Ability Activated on Actor: %s"), *GetNameSafe(GetAvatarActorFromActorInfo()));

    // 몬스터 캐릭터 가져오기
    ACMEnemyCharacterBase* Monster = Cast<ACMEnemyCharacterBase>(GetAvatarActorFromActorInfo());
    if (!Monster)
    {
        UE_LOG(LogTemp, Error, TEXT("[MonsterDeathDrop] Failed to cast AvatarActor to ACMEnemyCharacterBase."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!Monster->LootData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MonsterDeathDrop] Monster '%s' has no LootData assigned."), *GetNameSafe(Monster));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const TArray<FMonsterLootItem>& LootTable = Monster->LootData->LootItems;
    FVector MonsterLocation = Monster->GetActorLocation();
    
    UE_LOG(LogTemp, Log, TEXT("[MonsterDeathDrop] Processing Loot Table. Item Count: %d"), LootTable.Num());

    // 드랍 테이블 순회하며 주사위 굴리기
    for (const FMonsterLootItem& LootItem : LootTable)
    {
        if (!LootItem.ItemToDrop) 
        {
            continue;
        }

        // 0.0 ~ 1.0 사이의 랜덤 실수 생성
        float Roll = FMath::FRand(); 

        UE_LOG(LogTemp, Verbose, TEXT("[MonsterDeathDrop] Rolling for Item: %s. Chance: %f, Roll: %f"), *GetNameSafe(LootItem.ItemToDrop), LootItem.DropChance, Roll);

        // 확률 걸렸을때 
        if (Roll <= LootItem.DropChance)
        {
            int32 Quantity = FMath::RandRange(LootItem.DropQuantityRange.X, LootItem.DropQuantityRange.Y);
        	
            FTransform SpawnTransform;
            
            // 몬스터 위치에서 약간 위, 랜덤한 주변 위치로 설정하여 겹침 방지
            FVector RandomOffset = FMath::VRand() * 50.0f; 
            RandomOffset.Z = 50.0f;
            SpawnTransform.SetLocation(MonsterLocation + RandomOffset);
        	
            ACMPickupActor* Pickup = GetWorld()->SpawnActorDeferred<ACMPickupActor>(
                ACMPickupActor::StaticClass(), 
                SpawnTransform, 
                nullptr, 
                nullptr, 
                ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
            );

            if (Pickup)
            {
                Pickup->SetItemData(LootItem.ItemToDrop, Quantity);
            	
                UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);

            	// 서버에서 랜덤 벡터 계산
            	FVector RandomDirection = FMath::VRandCone(FVector::UpVector, FMath::DegreesToRadians(45.0f));
            	float ImpulseStrength = FMath::RandRange(300.0f, 500.0f);
            	FVector Impulse = RandomDirection * ImpulseStrength;
            	FVector RandomTorque = FMath::VRand() * FMath::RandRange(100.0f, 300.0f);

            	// 드랍 연출 실행 (계산된 값 전달)
            	Pickup->Multicast_PlayDropEffect(Impulse, RandomTorque);
            	
                UE_LOG(LogTemp, Log, TEXT("[MonsterDeathDrop] SUCCESS: Spawned Item '%s' (Quantity: %d) at %s"), 
                    *GetNameSafe(LootItem.ItemToDrop), Quantity, *SpawnTransform.GetLocation().ToString());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[MonsterDeathDrop] FAILED to spawn PickupActor for Item '%s'"), *GetNameSafe(LootItem.ItemToDrop));
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
