// Copyright Epic Games, Inc. All Rights Reserved.

#include "WildWorldGameMode.h"
#include "WildWorldCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWildWorldGameMode::AWildWorldGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
