#pragma once

#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"


class FEnchantedGraphCommands final : public TCommands<FEnchantedGraphCommands>
{
public:
	FEnchantedGraphCommands( )
		: TCommands<FEnchantedGraphCommands>(
			TEXT("EnchantedGraphCommands"),
			NSLOCTEXT("BlueprintEnchantments", "EnchantedGraphCommands", "Enchanted Graph Commands"),
			NAME_None,
			FEditorStyle::Get( ).GetStyleSetName( )
		) {}

	TSharedPtr<FUICommandInfo> SplitPinIntoExecs;
	TSharedPtr<FUICommandInfo> RecombinePinFromExecs;

	TSharedPtr<FUICommandInfo> SplitNextPinIntoExecs;
	TSharedPtr<FUICommandInfo> ReplaceNode;

	virtual void RegisterCommands( ) override;
};
