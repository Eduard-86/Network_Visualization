#pragma once

#include "K2Node.h"
#include "EdGraph/EdGraphNodeUtils.h"

#include "K2Node_UpdateDataTableRow.generated.h"

// Based on UK2Node_GetDataTableRow, significant amount of its code was copied as is

UCLASS(meta = (Keywords = "simona"))
class UK2Node_UpdateDataTableRow : public UK2Node
{
	GENERATED_UCLASS_BODY()

	//~ Begin UEdGraphNode Interface.
	virtual void  AllocateDefaultPins( ) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// Check comment for RefreshRowNameOptions. It was only connected with it
	// virtual void  PinDefaultValueChanged(UEdGraphPin * ChangedPin) override;
	virtual FText GetTooltipText( ) const override;
	virtual void  ExpandNode(class FKismetCompilerContext & CompilerContext,
	                         UEdGraph *                     SourceGraph) override;
	virtual FSlateIcon GetIconAndTint(FLinearColor & OutColor) const override;
	virtual void       PostReconstructNode( ) override;
	//~ End UEdGraphNode Interface.

	//~ Begin UK2Node Interface
	virtual bool IsNodeSafeToIgnore( ) const override { return true; }
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*> & OldPins) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar & ActionRegistrar) const override;
	virtual FText GetMenuCategory( ) const override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin * MyPin,
	                                    const UEdGraphPin * OtherPin,
	                                    FString & OutReason) const override;
	virtual void EarlyValidation(class FCompilerResultsLog & MessageLog) const override;
	virtual void PreloadRequiredAssets( ) override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin * Pin) override;
	//~ End UK2Node Interface

		/** Get the return type of our struct */
	UScriptStruct* GetReturnTypeForStruct();

	/** Get the then output pin */
	UEdGraphPin* GetThenPin() const;
	/** Get the Data Table input pin */
	UEdGraphPin* GetDataTablePin(const TArray<UEdGraphPin*>* InPinsToSearch = NULL) const;
	/** Get the spawn transform input pin */
	UEdGraphPin* GetRowNamePin() const;
	/** Get the result output pin */
	UEdGraphPin* GetRowDataPin(const TArray<UEdGraphPin*>* InPinsToSearch = NULL) const;

	/** Get the type of the TableRow to return */
	UScriptStruct* GetDataTableRowStructType() const;

private:
	/**
	 * Takes the specified "MutatablePin" and sets its 'PinToolTip' field (according
	 * to the specified description)
	 *
	 * @param   MutatablePin	The pin you want to set tool-tip text on
	 * @param   PinDescription	A string describing the pin's purpose
	 */
	void SetPinToolTip(UEdGraphPin& MutatablePin, const FText& PinDescription) const;

	/** Set the return type of our struct */
	void SetInputTypeForStruct(UScriptStruct* InClass);
	/** Queries for the authoritative return type, then modifies the return pin to match */
	void RefreshRowDataPinType();
	// UED Provides a special visual skin for this node. This node doesn't get that honor, and
	// doesn't really need as it can add new rows
	// Thus, method is not needed
	/** Triggers a refresh which will update the node's widget; aimed at updating the dropdown menu for the RowName input */
	// void RefreshRowNameOptions();

	/** Tooltip text for this node. */
	FText NodeTooltip;

	/** Constructing FText strings can be costly, so we cache the node's title */
	FNodeTextCache CachedNodeTitle;
};
