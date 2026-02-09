#include "UI/HUD/CMPlayerHUD.h"
#include "Character/Player/CMPlayerCharacterBase.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Game/CMGameStateMainStageV2.h"
#include "UI/HUD/CMPlayerStatusWidget.h"
#include "UI/HUD/CMLockOnWidget.h"
#include "UI/HUD/CMSkillSlotWidget.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "UI/HUD/Enemy/CMBossHealthWidget.h"
#include "Character/Enemy/CMEnemyCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "CMGameplayTags.h"
#include "Components/UI/CMQuickBarComponent.h"
#include "UI/HUD/CMQuickBarWidget.h"

UCMPlayerHUD::UCMPlayerHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetConfig.UILayer = EUILayerType::HUD;
	WidgetConfig.bShowMouseCursor = false;
	WidgetConfig.UIInputMode = EUIInputMode::GameOnly;
	WidgetConfig.DefaultVisibility = ESlateVisibility::SelfHitTestInvisible;
}

void UCMPlayerHUD::ToggleHelpWidget()
{
	if (HelpWidget)
	{
		if (HelpWidget->IsVisible())
		{
			HelpWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			HelpWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void UCMPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	if (HelpWidget)
	{
		HelpWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	ACMPlayerCharacterBase* PlayerCharacter = Cast<ACMPlayerCharacterBase>(GetOwningPlayerPawn());
	if (!IsValid(PlayerCharacter))
	{
		return;
	}

	UPawnUIComponent* PawnUIComp = PlayerCharacter->GetPawnUIComponent();
	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
	UPlayerCombatComponent* CombatComp = PlayerCharacter->GetPawnCombatComponent();
	UCMQuickBarComponent* QuickBarComp = PlayerCharacter->FindComponentByClass<UCMQuickBarComponent>();

	SkillSlots.Empty();
	SkillSlots.Emplace(CMGameplayTags::Player_Skill_Slot_Q, SkillSlot_Q);
	SkillSlots.Emplace(CMGameplayTags::Player_Skill_Slot_E, SkillSlot_E);
	SkillSlots.Emplace(CMGameplayTags::Player_Skill_Slot_R, SkillSlot_R);
	SkillSlots.Emplace(CMGameplayTags::Player_Skill_Slot_F, SkillSlot_F);

	if (ASC)
	{
		for (auto& CMSlot : SkillSlots)
		{
			if (UCMSkillSlotWidget* SkillWidget = CMSlot.Value)
			{
				SkillWidget->InitASC(ASC);
			}
		}
	}

	if (PawnUIComp && ASC)
	{
		// ASC 델리게이트 구독 초기화 (서버/클라이언트 모두 자동 작동)
		PawnUIComp->InitializeWithASC(ASC);

		PawnUIComp->OnAbilityIconSlot.AddDynamic(this, &ThisClass::OnAbilityIconSlotLoaded);
		PawnUIComp->BroadcastInitAbilityIcons();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to get UIComponent or ASC from PlayerCharacter!"), *GetName());
	}

	// UI 위젯 초기화
	if (PlayerStatusWidget && PawnUIComp)
	{
		PlayerStatusWidget->InitPlayerStatus(PawnUIComp);
	}

	if (LockOnWidget && CombatComp)
	{
		LockOnWidget->InitLockOnWidget(CombatComp);
	}

	InitBossHealthBar();

	if (QuickBarWidget && QuickBarComp)
	{
		QuickBarWidget->InitializeQuickBarComponent(QuickBarComp);
	}
}

void UCMPlayerHUD::NativeDestruct()
{
	Super::NativeDestruct();

	ACMGameStateMainStageV2* GameState = GetWorld()->GetGameState<ACMGameStateMainStageV2>();
	if (GameState)
	{
		if (GameState->OnBossSpawned.IsAlreadyBound(this, &ThisClass::InitBossHealthWidget))
		{
			GameState->OnBossSpawned.RemoveDynamic(this, &ThisClass::InitBossHealthWidget);
		}
	}
}

void UCMPlayerHUD::InitBossHealthBar()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	ACMGameStateMainStageV2* GameState = World->GetGameState<ACMGameStateMainStageV2>();

	if (GameState)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] GameState already exists. Initializing..."), *GetName());

		if (!GameState->OnBossSpawned.IsAlreadyBound(this, &ThisClass::InitBossHealthWidget))
		{
			GameState->OnBossSpawned.AddDynamic(this, &ThisClass::InitBossHealthWidget);
		}

		if (IsValid(GameState->GetCurrentBoss()))
		{
			InitBossHealthWidget(GameState->GetCurrentBoss());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] GameState is NULL. Waiting for GameStateSetEvent..."), *GetName());

		World->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateSet);
	}
}

void UCMPlayerHUD::OnGameStateSet(AGameStateBase* NewGameState)
{
	ACMGameStateMainStageV2* CastedGameState = Cast<ACMGameStateMainStageV2>(NewGameState);
	if (CastedGameState)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] GameState Arrived via Event! Initializing..."), *GetName());

		if (UWorld* World = GetWorld())
		{
			World->GameStateSetEvent.RemoveAll(this);
		}

		InitBossHealthBar();
	}
}

void UCMPlayerHUD::InitBossHealthWidget(ACMEnemyCharacterBase* NewBoss)
{
	if (BossHealthBar && IsValid(NewBoss))
	{
		BossHealthBar->BindToEnemy(NewBoss);
	}
}

void UCMPlayerHUD::OnAbilityIconSlotLoaded(FAbilityIconData InAbilityData)
{
    if (TObjectPtr<UCMSkillSlotWidget>* FoundWidget = SkillSlots.Find(InAbilityData.SlotTag))
    {
        if (*FoundWidget)
        {
            (*FoundWidget)->InitSkillSlotIcon(InAbilityData);
        }
    }
}
