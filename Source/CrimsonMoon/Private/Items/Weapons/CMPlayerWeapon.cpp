// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/CMPlayerWeapon.h"
#include "DataAssets/Weapon/CMDataAsset_WeaponData.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "Net/UnrealNetwork.h"

void ACMPlayerWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACMPlayerWeapon, WeaponData);
}

void ACMPlayerWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() && WeaponData && WeaponData->EquipGameplayEffect)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

		if (ASC)
		{
			// 이펙트 컨텍스트 생성
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			// 이벤트 스펙 생성
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(WeaponData->EquipGameplayEffect, 1.0f, ContextHandle);

			if (SpecHandle.IsValid())
			{
				EquipEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void ACMPlayerWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 저장해둔 이펙트 제거
	if (HasAuthority() && EquipEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(EquipEffectHandle, 1);
		}
	}

	Super::EndPlay(EndPlayReason);
}



void ACMPlayerWeapon::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InSpecHandles)
{
	GrantedAbilitySpecHandles = InSpecHandles;
}

const TArray<FGameplayAbilitySpecHandle>& ACMPlayerWeapon::GetGrantedAbilitySpecHandles() const
{
	return GrantedAbilitySpecHandles;
}