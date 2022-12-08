#pragma once

#include "EdGraphUtilities.h"
#include "SGraphNode.h"

struct FEnchantedK2NodeFactory final : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(class UEdGraphNode* InNode) const override;
};