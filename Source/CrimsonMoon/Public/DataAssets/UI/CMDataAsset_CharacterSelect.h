#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CMDataAsset_CharacterSelect.generated.h"

USTRUCT(BlueprintType)
struct FCharacterSelectInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CharacterClassTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText CharacterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> CharacterImage;
};

UCLASS()
class CRIMSONMOON_API UCMDataAsset_CharacterSelect : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select")
	TArray<FCharacterSelectInfo> CharacterOptions;
};
