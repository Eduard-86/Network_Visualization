#include "GraphCommands.h"

#define LOCTEXT_NAMESPACE "BlueprintEnchantments"

void FEnchantedGraphCommands::RegisterCommands( )
{
	UI_COMMAND(
		SplitPinIntoExecs,
		"Split Pin into Execs",
		"Replaces data pin with an embedded branch/switch node",
		EUserInterfaceActionType::Button,
		FInputChord()
	);
	UI_COMMAND(
		RecombinePinFromExecs,
		"Recombine Pin from Execs",
		"Replaces embedded branch/switch node with a data pin",
		EUserInterfaceActionType::Button,
		FInputChord()
	);
	UI_COMMAND(
		SplitNextPinIntoExecs,
		"Split Next Pin into Execs",
		"Replaces next data pin with an embedded branch/switch node, or recombines last splitted if no more options left",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::B)
	);
	UI_COMMAND(
		ReplaceNode,
		"Replace Node",
		"Replaces one with another, preserving as many pins as possible",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::H, EModifierKey::Control)
	);
}

#undef LOCTEXT_NAMESPACE
