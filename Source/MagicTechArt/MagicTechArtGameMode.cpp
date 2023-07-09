// Copyright Epic Games, Inc. All Rights Reserved.

#include "MagicTechArtGameMode.h"
#include "MagicTechArtHUD.h"
#include "MagicTechArtCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMagicTechArtGameMode::AMagicTechArtGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMagicTechArtHUD::StaticClass();
}
