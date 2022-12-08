#include "AdvancedK2NodeFactory.h"

#include "K2Node_CallFunction.h"
#include "K2Node_MacroInstance.h"
#include "SGraphNodeK2Enchanted.h"

TSharedPtr<SGraphNode> FEnchantedK2NodeFactory::CreateNode(UEdGraphNode * InNode) const
{
	if (InNode->GetClass( ) == UK2Node_CallFunction::StaticClass( ))
	{
		return SNew(SGraphNodeK2Enchanted, (UK2Node_CallFunction*)InNode);
	}
	else if (UK2Node_MacroInstance * MacroNode = Cast<UK2Node_MacroInstance>(InNode))
	{
		return SNew(SGraphNodeK2Enchanted, MacroNode);
	}

	return nullptr;
}
