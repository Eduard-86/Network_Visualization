#pragma once

#include "K2Node.h"

#include "K2Node_BuildTypeSwitch.generated.h"

UCLASS(meta = (Keywords = "if switch select branch build simona"))
class UK2Node_BuildTypeSwitch final : public UK2Node
{
	GENERATED_UCLASS_BODY( )

	//~ Begin UEdGraphNode Interface.
	virtual void       AllocateDefaultPins( ) override;
	virtual FText      GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText      GetTooltipText( ) const override;
	virtual void       ExpandNode(class FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph) override;
	virtual FSlateIcon GetIconAndTint(FLinearColor & OutColor) const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UK2Node Interface
	virtual void  GetMenuActions(FBlueprintActionDatabaseRegistrar & ActionRegistrar) const override;
	virtual FText GetMenuCategory( ) const override;
	//~ End UK2Node Interface

private:
	UEdGraphPin * GetThenDevelopmentPin( ) const;
	UEdGraphPin * GetThenShippingPin( ) const;
};
