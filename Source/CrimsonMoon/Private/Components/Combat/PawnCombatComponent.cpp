// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Combat/PawnCombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CMGameplayTags.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Items/Weapons/CMWeaponBase.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraComponent.h"
#include "DataAssets/Weapon/CMDataAsset_WeaponData.h"
#include "Items/Weapons/CMPlayerWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Structs/CMStructTypes.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "CMFunctionLibrary.h"

void UPawnCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1초 후 비동기 파티클 프리로딩 시작 (무기 등록 완료 후)
	FTimerHandle InitTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		InitTimerHandle,
		this,
		&ThisClass::StartAsyncParticlePreloading,
		1.0f,
		false
		);
}

void UPawnCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPawnCombatComponent, CharacterCarriedWeaponList);
	DOREPLIFETIME(UPawnCombatComponent, CurrentEquippedWeaponTag);
}

void UPawnCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	// 공격때마다 1회만 공격처리 (중복 방지)
	if (OverlappedActors.Contains(HitActor))
	{
		return;
	}

	OverlappedActors.AddUnique(HitActor);

	// GAS 이벤트 전송 (공통 로직)
	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn();
	EventData.Target = HitActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		CMGameplayTags::Shared_Event_MeleeHit,
		EventData
	);

	// 자식 클래스의 추가 로직 실행
	OnHitTargetActorImpl(HitActor);
}

void UPawnCombatComponent::OnHitTargetActorImpl(AActor* HitActor)
{
	// 기본 구현은 비어있음 - 자식 클래스에서 필요시 override
}

void UPawnCombatComponent::OnWeaponPulledFromTargetActor(AActor* InteractingActor) {}

void UPawnCombatComponent::RegisterSpawnedWeapon(FGameplayTag InWeaponTagToResister, ACMWeaponBase* InWeaponToResister, bool bResisterAsEquippedWeapon)
{
	checkf(GetCharacterCarriedWeaponByTag(InWeaponTagToResister) == nullptr, TEXT("%s는(은) 이미 장착된 무기 목록에 포함되어 있습니다"), *InWeaponTagToResister.ToString());

	check(InWeaponToResister);

	FReplicatedWeaponEntry ReplicatedWeaponEntry;
	ReplicatedWeaponEntry.WeaponTag = InWeaponTagToResister;
	ReplicatedWeaponEntry.WeaponActor = InWeaponToResister;
	CharacterCarriedWeaponList.Add(ReplicatedWeaponEntry);

	InWeaponToResister->OnWeaponHitTarget.BindUObject(this, &ThisClass::OnHitTargetActor);
	InWeaponToResister->OnWeaponPulledFromTarget.BindUObject(this, &ThisClass::OnWeaponPulledFromTargetActor);

	if (bResisterAsEquippedWeapon)
	{
		CurrentEquippedWeaponTag = InWeaponTagToResister;
	}
}

ACMWeaponBase* UPawnCombatComponent::GetCharacterCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const
{
	for (const FReplicatedWeaponEntry& Entry : CharacterCarriedWeaponList)
	{
		if (Entry.WeaponTag == InWeaponTagToGet)
		{
			return Entry.WeaponActor;
		}
	}

	return nullptr;
}

ACMWeaponBase* UPawnCombatComponent::GetCharacterCurrentEquippedWeapon() const
{
	if (!CurrentEquippedWeaponTag.IsValid())
	{
		return nullptr;
	}

	return GetCharacterCarriedWeaponByTag(CurrentEquippedWeaponTag);
}

void UPawnCombatComponent::SetCurrentEquippedWeaponTag(const FGameplayTag& NewWeaponTag)
{
	// 이 함수는 서버에서만 호출되어야 함
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	const FGameplayTag OldWeaponTag = CurrentEquippedWeaponTag;
	if (OldWeaponTag == NewWeaponTag)
	{
		return;
	}

	// 1. 서버에서 태그 값을 변경 (이 값은 원격 클라이언트로 복제됨)
	CurrentEquippedWeaponTag = NewWeaponTag;

	//2. 서버(Listen)에서 즉시 시각적/물리적 상태를 적용
	HandleClientSideEquipEffects(CurrentEquippedWeaponTag, OldWeaponTag);

	// 3. 원격 클라이언트들은 OnRep_CurrentEquippedWeaponTag를 통해 HandleClientSideEquipEffects가 정상적으로 호출
}

void UPawnCombatComponent::ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
	if (const APawn* OwningPawn = Cast<APawn>(GetOwner()))
	{
		if (!OwningPawn->HasAuthority())
		{
			if (OwningPawn->IsLocallyControlled())
			{
				ServerToggleCollision(bShouldEnable, ToggleDamageType);
			}
			return;
		}
	}

	// 서버에서만 실제 콜리전 변경
	HandleToggleCollision(bShouldEnable, ToggleDamageType);
}

ACMCharacterBase* UPawnCombatComponent::GetOwnerCharacter() const
{
	if (ACMCharacterBase* OwningCharacter = Cast<ACMCharacterBase>(GetOwner()))
	{
		return OwningCharacter;
	}

	return nullptr;
}

void UPawnCombatComponent::HandleToggleCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (ToggleDamageType == EToggleDamageType::CurrentEquippedWeapon)
	{
		const ACMWeaponBase* WeaponToCollision = GetCharacterCurrentEquippedWeapon();

		if (!WeaponToCollision)
		{
			return;
		}

		const ECollisionEnabled::Type CollisionEnabled = bShouldEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;
		WeaponToCollision->GetWeaponCollisionBox()->SetCollisionEnabled(CollisionEnabled);

		if (!bShouldEnable)
		{
			OverlappedActors.Empty();
		}
	}
}

void UPawnCombatComponent::OnRep_CharacterCarriedWeaponList()
{
	// 무기 리스트가 복제된 후, 장착된 무기가 있다면 클라이언트 효과를 적용
	if (!GetOwner()->HasAuthority() && CurrentEquippedWeaponTag.IsValid())
	{
		HandleClientSideEquipEffects(CurrentEquippedWeaponTag, FGameplayTag::EmptyTag);
	}
}

void UPawnCombatComponent::OnRep_CurrentEquippedWeaponTag(const FGameplayTag& OldWeaponTag)
{
	// 서버가 CurrentEquippedWeaponTag를 변경하면, 모든 클라이언트에서 이 함수가 호출
	HandleClientSideEquipEffects(CurrentEquippedWeaponTag, OldWeaponTag);
}

void UPawnCombatComponent::HandleClientSideEquipEffects(const FGameplayTag& NewWeaponTag, const FGameplayTag& OldWeaponTag)
{
	const ACMCharacterBase* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return;
	}

	if (GetOwner()->HasAuthority() && OldWeaponTag.IsValid())
	{
		if (const ACMPlayerWeapon* OldWeapon = Cast<ACMPlayerWeapon>(GetCharacterCarriedWeaponByTag(OldWeaponTag)))
		{
			if (const UCMDataAsset_WeaponData* WeaponData = OldWeapon->WeaponData)
			{
				if (UPawnUIComponent* PawnUIComp = OwnerCharacter->GetPawnUIComponent())
				{
					for (const FCMPlayerAbilitySet& AbilitySet : WeaponData->DefaultWeaponAbilities)
					{
						PawnUIComp->RemoveAbilityIcon(AbilitySet.SlotTag);
					}
				}
			}
		}
	}

	// 1. 이전 무기가 있었다면, 클라이언트 효과(입력, 애님)를 제거
	if (OldWeaponTag.IsValid())
	{
		if (ACMPlayerWeapon* OldWeapon = Cast<ACMPlayerWeapon>(GetCharacterCarriedWeaponByTag(OldWeaponTag)))
		{
			const UCMDataAsset_WeaponData* WeaponData = OldWeapon->WeaponData;

			// 애님 레이어 해제
			if (WeaponData->WeaponAnimLayerToLink)
			{
				OwnerCharacter->GetMesh()->UnlinkAnimClassLayers(WeaponData->WeaponAnimLayerToLink);
			}

			// [로컬 플레이어 전용] 입력 컨텍스트 제거
			if (OwnerCharacter->IsLocallyControlled())
			{
				if (APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController()))
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
					{
						if (WeaponData->WeaponInputMappingContext)
						{
							Subsystem->RemoveMappingContext(WeaponData->WeaponInputMappingContext);
						}
					}
				}
			}

			// 무기 장착 제거
			if (WeaponData->UnequippedSocketName != NAME_None)
			{
				OldWeapon->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData->UnequippedSocketName);
			}

			// 무기 숨기기
			OldWeapon->HideWeapon();
		}
	}

	// 2. 새 무기가 있다면, 클라이언트 효과(입력, 애님)를 적용
	if (NewWeaponTag.IsValid())
	{
		if (ACMPlayerWeapon* NewWeapon = Cast<ACMPlayerWeapon>(GetCharacterCarriedWeaponByTag(NewWeaponTag)))
		{
			const UCMDataAsset_WeaponData* WeaponData = NewWeapon->WeaponData;

			// 애님 레이어 연결
			if (WeaponData->WeaponAnimLayerToLink)
			{
				OwnerCharacter->GetMesh()->LinkAnimClassLayers(WeaponData->WeaponAnimLayerToLink);
			}

			// [로컬 플레이어 전용] 입력 컨텍스트 추가
			if (OwnerCharacter->IsLocallyControlled())
			{
				if (const APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController()))
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
					{
						if (WeaponData->WeaponInputMappingContext)
						{
							Subsystem->AddMappingContext(WeaponData->WeaponInputMappingContext, 1);
						}
					}
				}
			}

			// 무기를 Equipped 소켓(손)에 부착
			if (WeaponData->EquippedSocketName != NAME_None)
			{
				USkeletalMeshComponent* AttachTargetMesh;

				// TODO: 성능 저하 이슈 발생시 BeginPlay에서 캐싱하도록 수정 필요
				// 1. 캐싱된 메시가 유효한지 확인 (이미 찾은 적이 있다면 바로 사용)
				if (CachedWeaponMesh.IsValid())
				{
					AttachTargetMesh = CachedWeaponMesh.Get();
				}
				else
				{
					// 2. CMFunctionLibrary를 사용하여 메시 찾기
					AttachTargetMesh = UCMFunctionLibrary::FindSkeletalMeshByTag(
						OwnerCharacter,
						TargetMeshTagName,
						true // 못 찾으면 기본 메시 반환
						);

					// 3. 찾은 메시를 캐싱
					if (AttachTargetMesh)
					{
						CachedWeaponMesh = AttachTargetMesh;
					}
				}

				// 4. 결정된 대상에 무기 부착
				if (AttachTargetMesh)
				{
					NewWeapon->AttachToComponent(AttachTargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData->EquippedSocketName);
				}
			}

			// 무기 보이기
			NewWeapon->ShowWeapon();

			// 스킬 파티클 시스템 프라이밍 (첫 사용 시 프리징 방지)
			PreloadSkillParticles(WeaponData);
		}
	}
}

void UPawnCombatComponent::PreloadSkillParticles(const UCMDataAsset_WeaponData* WeaponData)
{
	if (!WeaponData || WeaponData->SkillParticleSystems.IsEmpty())
	{
		return;
	}

	// 각 파티클을 화면 밖에서 매우 작은 스케일로 스폰하여 메모리에 로드
	for (const TSoftObjectPtr<UNiagaraSystem>& ParticlePtr : WeaponData->SkillParticleSystems)
	{
		if (UNiagaraSystem* ParticleSystem = ParticlePtr.LoadSynchronous())
		{
			// 화면 밖 위치에 거의 보이지 않는 크기로 스폰
			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				ParticleSystem,
				FVector(0, 0, -10000), // 화면 밖
				FRotator::ZeroRotator,
				FVector(0.001f), // 거의 보이지 않는 크기
				true,            // Auto Destroy
				true,            // Auto Activate
				ENCPoolMethod::None,
				false // Preculling
				);

			// 즉시 비활성화하여 메모리에만 로드된 상태 유지
			if (NiagaraComponent)
			{
				NiagaraComponent->DeactivateImmediate();
			}
		}
	}
}

void UPawnCombatComponent::ServerToggleCollision_Implementation(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
	HandleToggleCollision(bShouldEnable, ToggleDamageType);
}

// TODO: 별도의 컴
void UPawnCombatComponent::StartAsyncParticlePreloading()
{
	// 모든 등록된 무기의 파티클 시스템을 수집
	PendingParticlesToLoad.Empty();
	CurrentParticleLoadIndex = 0;

	for (const FReplicatedWeaponEntry& Entry : CharacterCarriedWeaponList)
	{
		if (const ACMPlayerWeapon* PlayerWeapon = Cast<ACMPlayerWeapon>(Entry.WeaponActor))
		{
			if (const UCMDataAsset_WeaponData* WeaponData = PlayerWeapon->WeaponData)
			{
				// 각 무기의 파티클 시스템을 로딩 대기 리스트에 추가
				for (const TSoftObjectPtr<UNiagaraSystem>& ParticlePtr : WeaponData->SkillParticleSystems)
				{
					PendingParticlesToLoad.AddUnique(ParticlePtr);
				}
			}
		}
	}

	if (!PendingParticlesToLoad.IsEmpty())
	{
		// 0.1초마다 하나씩 로드 (프레임 분산)
		GetWorld()->GetTimerManager().SetTimer(
			ParticlePreloadTimerHandle,
			this,
			&ThisClass::LoadNextParticleAsync,
			0.1f,
			true // 반복
			);
	}
}

void UPawnCombatComponent::LoadNextParticleAsync()
{
	if (CurrentParticleLoadIndex >= PendingParticlesToLoad.Num())
	{
		// 모든 파티클 로딩 완료
		GetWorld()->GetTimerManager().ClearTimer(ParticlePreloadTimerHandle);
		PendingParticlesToLoad.Empty();
		return;
	}

	const TSoftObjectPtr<UNiagaraSystem>& ParticlePtr = PendingParticlesToLoad[CurrentParticleLoadIndex];

	// 이미 로드된 경우 바로 프라이밍
	if (UNiagaraSystem* ParticleSystem = ParticlePtr.Get())
	{
		// 화면 밖 위치에 거의 보이지 않는 크기로 스폰
		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ParticleSystem,
			FVector(0, 0, -10000),
			FRotator::ZeroRotator,
			FVector(0.001f),
			true,
			true,
			ENCPoolMethod::None,
			false
			);

		// 즉시 비활성화
		if (NiagaraComponent)
		{
			NiagaraComponent->DeactivateImmediate();
		}
	}
	else
	{
		// 아직 로드되지 않았으면 동기 로딩 (하지만 타이머로 분산되어 렉 감소)
		if (UNiagaraSystem* LoadedSystem = ParticlePtr.LoadSynchronous())
		{
			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				LoadedSystem,
				FVector(0, 0, -10000),
				FRotator::ZeroRotator,
				FVector(0.001f),
				true,
				true,
				ENCPoolMethod::None,
				false
				);

			if (NiagaraComponent)
			{
				NiagaraComponent->DeactivateImmediate();
			}
		}
	}

	CurrentParticleLoadIndex++;
}