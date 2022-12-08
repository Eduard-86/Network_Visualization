#pragma once

#include "K2Node_MakeCompilerMessage.generated.h"

UENUM( )
enum ECompilerMessageType
{
	Note,
	Warning,
	Error
};


UCLASS( )
class UK2Node_MakeCompilerMessage final : public UK2Node
{
	GENERATED_BODY( )

	//~ Begin UEdGraphNode Interface.
	virtual void       AllocateDefaultPins( ) override;
	virtual FText      GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText      GetTooltipText( ) const override;
	virtual void       PinConnectionListChanged(UEdGraphPin * Pin) override;
	virtual bool       IsNodePure() const override;
	virtual void       ExpandNode(class FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph) override;
	virtual FSlateIcon GetIconAndTint(FLinearColor & OutColor) const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UK2Node Interface
	virtual void  GetMenuActions(FBlueprintActionDatabaseRegistrar & ActionRegistrar) const override;
	virtual FText GetMenuCategory( ) const override;
	//~ End UK2Node Interface
};
