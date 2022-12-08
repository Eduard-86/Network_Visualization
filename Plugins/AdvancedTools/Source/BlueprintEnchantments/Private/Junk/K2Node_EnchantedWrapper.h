#pragma once

#include "K2Node.h"

#include "K2Node_EnchantedWrapper.generated.h"

UCLASS( )
class UK2Node_EnchantedWrapper : public UK2Node
{
	GENERATED_BODY( )

public:
	void WrapNode(UK2Node * Node);

	// UObject interface
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void Serialize(FArchive & Ar) override;
	// End of UObject interface

	// UEdGraphNode interface
	virtual void GetPinHoverText(const UEdGraphPin & Pin, FString & HoverTextOut) const override;
	virtual void AllocateDefaultPins( ) override;
	virtual FLinearColor GetNodeTitleColor( ) const override;
	virtual FText GetTooltipText( ) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FString GetDescriptiveCompiledName( ) const override;
	virtual bool HasDeprecatedReference( ) const override;
	virtual FEdGraphNodeDeprecationResponse GetDeprecationResponse
	(EEdGraphNodeDeprecationType DeprecationType) const override;
	virtual void PostPlacedNewNode( ) override;
	virtual FString GetDocumentationLink( ) const override;
	virtual FString GetDocumentationExcerptName( ) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor & OutColor) const override;
	virtual bool CanPasteHere(const UEdGraph * TargetGraph) const override;
	virtual void PinDefaultValueChanged(UEdGraphPin * Pin) override;
	virtual void AddSearchMetaDataInfo(TArray<struct FSearchTagDataPair> & OutTaggedMetaData) const override;
	virtual TSharedPtr<SWidget> CreateNodeImage( ) const override;
	virtual UObject * GetJumpTargetForDoubleClick( ) const override;
	virtual bool CanJumpToDefinition( ) const override;
	virtual void JumpToDefinition( ) const override;
	// End of UEdGraphNode interface

	// UK2Node interface
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin *> & OldPins) override;
	virtual bool IsNodePure( ) const override;
	virtual bool HasExternalDependencies(TArray<class UStruct *> * OptionalOutput) const override;
	virtual void PostReconstructNode( ) override;
	virtual bool ShouldDrawCompact( ) const override;
	virtual bool ShouldDrawAsBead( ) const override;
	virtual FText GetCompactNodeTitle( ) const override;
	virtual void PostPasteNode( ) override;
	virtual void ValidateNodeDuringCompilation(class FCompilerResultsLog & MessageLog) const override;
	virtual bool ShouldShowNodeProperties( ) const override;
	virtual void GetRedirectPinNames(const UEdGraphPin & Pin, TArray<FString> & RedirectPinNames) const override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin * Pin) override;
	virtual class FNodeHandlingFunctor * CreateNodeHandler
	(class FKismetCompilerContext & CompilerContext) const override;
	virtual void  ExpandNode(class FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph) override;
	virtual FName GetCornerIcon( ) const override;
	virtual FText GetToolTipHeading( ) const override;
	virtual void  GetNodeAttributes(TArray<TKeyValuePair<FString, FString>> & OutNodeAttributes) const override;
	virtual FText GetMenuCategory( ) const override;
	virtual bool  IsActionFilteredOut(class FBlueprintActionFilter const & Filter) override;
	virtual bool  IsConnectionDisallowed
	(const UEdGraphPin * MyPin, const UEdGraphPin * OtherPin, FString & OutReason) const override;
	// End of UK2Node interface

private:
	void          ImportPins( );
	UEdGraphPin & RemapPin(const UEdGraphPin & Pin) const;
	UEdGraphPin * RemapPin(const UEdGraphPin * Pin) const;

private:
	UPROPERTY( )
	FName ExpandedPinName;

	UPROPERTY( )
	UK2Node * WrappedNode;
};
