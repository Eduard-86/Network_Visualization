#pragma once

#include "EnchantedNode.h"
#include "K2Node_CommutativeAssociativeBinaryOperator.h"

#include "K2Node_CommutativeAssociativeBinaryOperatorEnchanted.generated.h"

UCLASS( )
class UK2Node_CommutativeAssociativeBinaryOperatorEnchanted final : public UK2Node_CommutativeAssociativeBinaryOperator,
                                                                    public IEnchantedNode
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
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin *> & OldPins) override;
	virtual void ExpandNode(FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph) override;
	// @formatter:wrap_lines restore

	/**
	 * Pure nodes without data links are always pruned
	 * We are adding additional exec pins when there are some, and even more if there are none
	 * So it's never pure
	 * Originally UK2Node_CallFunction returns function purity flag, but better not influence its
	 * algorithms, and not modify it
	 */
	virtual bool IsNodePure( ) const override { return false; }

	// Original is not satisfactory
public:
	virtual void AddInputPin( ) override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog & MessageLog) const override;

	// VTable hacks only
public:
	virtual void GetNodeContextMenuActions(UToolMenu * Menu, UGraphNodeContextMenuContext * Context) const override;
	virtual bool CanAddPin( ) const override;
#if ENGINE_MAJOR_VERSION >= 5
	virtual void RemoveInputPin(UEdGraphPin * Pin) override;
protected:
	virtual bool CanRemovePin(const UEdGraphPin * Pin) const override;
#endif

	// Reimplementations
private:
	constexpr static int32 BinaryOperatorInputsNum = 2;
	FEdGraphPinType        GetType( ) const;
	void                   AddInputPinInner(int32 AdditionalPinIndex);
	static FName           GetNameForPin(int32 PinIndex);
	static int32           GetMaxInputPinsNum( );

	// ~Overrides

private:
	static ThisClass & DoEnchant(const UK2Node_CommutativeAssociativeBinaryOperator & Node);
	static void        AddMissingPins(const Super & Sample, ThisClass & WhomToAdd);
	static void        AddMissingPins(const ThisClass & Sample, Super & WhomToAdd);
	static void        DoAddMissingPins(IK2Node_AddPinInterface & WhomToAdd, int32 NumPins);
	void               DoExpandPin( );

	bool IsInBadState( ) const;
	void SetBadState( );
	void ResetBadState( );
};
