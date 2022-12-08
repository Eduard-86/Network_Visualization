#pragma once

#include "EnchantedNode.h"
#include "K2Node_CallFunction.h"

#include "K2Node_CallFunctionEnchanted.generated.h"

UCLASS( )
class UK2Node_CallFunctionEnchanted final : public UK2Node_CallFunction, public IEnchantedNode
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
	// @formatter:wrap_lines false
	virtual void AllocateDefaultPins( ) override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin * MyPin, const UEdGraphPin * OtherPin, FString & OutReason) const override;
	virtual void ExpandNode(FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph) override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog & MessageLog) const override;
	virtual void GetPinHoverText(const UEdGraphPin & Pin, FString & HoverTextOut) const override;
	// @formatter:wrap_lines restore

	/**
	 * Pure nodes without data links are always pruned
	 * We are adding additional exec pins when there are some, and even more if there are none
	 * So it's never pure
	 * Originally UK2Node_CallFunction returns function purity flag, but better not influence its
	 * algorithms, and not modify it
	 */
	virtual bool IsNodePure( ) const override { return false; }

protected:
	friend class FGraphEnchantments;
	virtual ERenamePinResult RenameUserDefinedPinImpl(const FName OldName, const FName NewName, bool bTest) override;

	// ~Overrides

private:
	static UK2Node_CallFunctionEnchanted & DoEnchant(const UK2Node_CallFunction & Node, const UEdGraphPin & Pin);

	static bool IsConstructableFrom(const UK2Node_CallFunction & Node, const UEdGraphPin & Pin);
	static bool IsConstructableFrom(const UK2Node_CallFunction & Node);

	static UEdGraphPin * GetPinToConstructFrom(const UK2Node & Node);
	static UEdGraphPin * GetPinToConstructFrom(const UK2Node_CallFunction & Node);

	void ExpandPin(FName PinName);
	void ExpandPin(const UEdGraphPin & Pin);
	void DoExpandPin( );

	bool IsInBadState() const;
	void SetBadState();
	void ResetBadState();

private:
	UPROPERTY( )
	FName ExpandedPinName;

	UPROPERTY( )
	FName NameOfPinAfterExpandedOne;

	mutable bool bPinTooltipsValid;
};
