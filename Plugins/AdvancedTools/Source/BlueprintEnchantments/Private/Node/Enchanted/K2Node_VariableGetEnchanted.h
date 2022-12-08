#pragma once

#include "EnchantedNode.h"
#include "K2Node_VariableGet.h"

#include "K2Node_VariableGetEnchanted.generated.h"

/*
 * Check UK2Node_CallFunctionEnchanted for comments
 */

UCLASS( )
class UK2Node_VariableGetEnchanted final : public UK2Node_VariableGet, public IEnchantedNode
{
	GENERATED_BODY( )

	// IEnchantedNode
public:
	virtual bool                 CanEnchant(const UK2Node & Node) const override;
	virtual UK2Node *            Enchant(const UK2Node & Node) const override;
	virtual TOptional<UK2Node *> Enchant(UK2Node & Node, const UEdGraphPin & Pin) const override;
	virtual UK2Node &            Disenchant( ) const override;
	virtual UK2Node *            SplitNextPin( ) override;
	// ~IEnchantedNode

	// Overrides
public:
	virtual void AllocateDefaultPins( ) override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin *> & OldPins) override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;
	virtual void ExpandNode(FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph) override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog & MessageLog) const override;

	virtual bool IsNodePure( ) const override { return false; }
	// ~Overrides

private:
	static bool        IsConstructableFrom(const UK2Node_VariableGet & Node);
	static ThisClass & DoEnchant(const Super & Node);
	void               ExpandPin( );

	bool IsInBadState() const;
	void SetBadState();
	void ResetBadState();
};
