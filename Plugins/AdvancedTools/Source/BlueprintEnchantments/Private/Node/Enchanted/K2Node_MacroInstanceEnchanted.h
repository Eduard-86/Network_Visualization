#pragma once

#include "EnchantedNode.h"
#include "K2Node_MacroInstance.h"

#include "K2Node_MacroInstanceEnchanted.generated.h"

UCLASS( )
class UK2Node_MacroInstanceEnchanted final : public UK2Node_MacroInstance, public IEnchantedNode
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

protected:
	virtual ERenamePinResult RenameUserDefinedPinImpl(const FName OldName, const FName NewName, bool bTest) override;

	// Overrides
public:
	// @formatter:wrap_lines false
	virtual void AllocateDefaultPins( ) override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin * MyPin, const UEdGraphPin * OtherPin, FString & OutReason) const override;
	virtual void ExpandNode(FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph) override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog & MessageLog) const override;
	virtual bool ShouldMergeChildGraphs( ) const override;
	virtual void NodeConnectionListChanged( ) override;
	// @formatter:wrap_lines restore

	/**
	 * Pure nodes without data links are always pruned
	 * We are adding additional exec pins when there are some, and even more if there are none
	 * So it's never pure
	 * Originally UK2Node_CallFunction returns function purity flag, but better not influence its
	 * algorithms, and not modify it
	 */
	virtual bool IsNodePure( ) const override { return false; }

	// SO MUCH LINKER ERR FIXES
	virtual void Serialize(FArchive & Ar) override;
	virtual void PreloadRequiredAssets( ) override;
	virtual FText GetTooltipText( ) const override;
	virtual FText GetKeywords( ) const override;
	virtual void PostPasteNode( ) override;
	virtual FLinearColor GetNodeTitleColor( ) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void GetNodeContextMenuActions(UToolMenu * Menu, UGraphNodeContextMenuContext * Context) const override;
	virtual FString GetDocumentationLink( ) const override;
	virtual FString GetDocumentationExcerptName( ) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor & OutColor) const override;
	virtual bool CanPasteHere(const UEdGraph * TargetGraph) const override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin * Pin) override;
	virtual void PostReconstructNode( ) override;
	virtual bool HasExternalDependencies(TArray<UStruct *> * OptionalOutput) const override;
	virtual void GetNodeAttributes(TArray<TKeyValuePair<FString, FString>> & OutNodeAttributes) const override;
	virtual FText GetMenuCategory( ) const override;
	virtual FBlueprintNodeSignature GetSignature( ) const override;
	virtual bool IsActionFilteredOut(FBlueprintActionFilter const & Filter) override;
	virtual FText GetCompactNodeTitle( ) const override;
	virtual bool ShouldDrawCompact( ) const override;
	virtual FName GetCornerIcon( ) const override;
	virtual void PostFixupAllWildcardPins(bool bInAllWildcardPinsUnlinked) override;
	// ~Overrides

private:
	static ThisClass & DoEnchant(const Super & Node, const UEdGraphPin & Pin);

	static bool IsConstructableFrom(const Super & Node, const UEdGraphPin & Pin);
	static bool IsConstructableFrom(const Super & MacroNode);

	static UEdGraphPin * GetPinToConstructFrom(const UK2Node & Node);
	static UEdGraphPin * GetPinToConstructFrom(const UK2Node_MacroInstance & Node);

	void ExpandPin(FName PinName);
	void ExpandPin(const UEdGraphPin & Pin);
	void DoExpandPin( );

	static UEdGraphPin * FindExecutionPin(const UEdGraphNode & Node);
	static UEdGraphPin * FindThenPin(const UEdGraphNode & Node);
	static UEdGraphPin * DoFindPin(
		const UEdGraphNode &       Node,
		const EEdGraphPinDirection Direction,
		const FName                PinCategory,
		const FName                DesiredName
	);

	bool IsInBadState( ) const;
	void SetBadState( );
	void ResetBadState( );

private:
	UPROPERTY( )
	FName ExpandedPinName;

	UPROPERTY( )
	FName NameOfPinAfterExpandedOne;
};
