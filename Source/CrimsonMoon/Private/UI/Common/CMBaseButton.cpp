#include "UI/Common/CMBaseButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

void UCMBaseButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BaseButton)
	{	
		BaseButton->OnClicked.AddDynamic(this, &UCMBaseButton::OnButtonClicked);
		BaseButton->OnHovered.AddDynamic(this, &UCMBaseButton::OnButtonHovered);
	}

	SetButtonText(FText::GetEmpty());
}

void UCMBaseButton::OnButtonClicked()
{
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ClickSound);
	}

	OnClicked.Broadcast();
}

void UCMBaseButton::OnButtonHovered()
{
	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), HoverSound);
	}
}

void UCMBaseButton::SetButtonText(const FText& InText)
{
	if (ButtonText)
	{
		ButtonText->SetText(InText);
	}
}