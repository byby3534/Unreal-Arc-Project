// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MainMenu/CMGameModeMainMenu.h"
#include "Controllers/CMMainMenuPlayerController.h"

ACMGameModeMainMenu::ACMGameModeMainMenu()
{
	PlayerControllerClass = ACMMainMenuPlayerController::StaticClass();

	DefaultPawnClass = nullptr;

	HUDClass = nullptr;
}