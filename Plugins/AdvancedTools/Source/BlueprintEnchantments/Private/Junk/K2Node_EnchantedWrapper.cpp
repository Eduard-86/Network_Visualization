#include "K2Node_EnchantedWrapper.h"

#include "AssumeChecked.h"
#include "EdGraph/EdGraphPin.h"
#include "Textures/SlateIcon.h"

void UK2Node_EnchantedWrapper::WrapNode(UK2Node * Node)
{
	WrappedNode = Node;
	ImportPins( );
}

void UK2Node_EnchantedWrapper::PostDuplicate(bool bDuplicateForPIE)
{
	if (WrappedNode)WrappedNode->PostDuplicate(bDuplicateForPIE);
}

void UK2Node_EnchantedWrapper::Serialize(FArchive & Ar)
{
	// if (WrappedNode) WrappedNode->Serialize(Ar);
}

void UK2Node_EnchantedWrapper::GetPinHoverText(const UEdGraphPin & Pin, FString & HoverTextOut) const
{
	if (WrappedNode) WrappedNode->GetPinHoverText(RemapPin(Pin), HoverTextOut);
}

void UK2Node_EnchantedWrapper::AllocateDefaultPins( )
{
	if (WrappedNode)
	{
		WrappedNode->AllocateDefaultPins( );
		ImportPins( );
	}
}

FLinearColor UK2Node_EnchantedWrapper::GetNodeTitleColor( ) const
{
	return WrappedNode ? WrappedNode->GetNodeTitleColor( ) : FLinearColor( );
}

FText UK2Node_EnchantedWrapper::GetTooltipText( ) const
{
	return WrappedNode ? WrappedNode->GetTooltipText( ) : FText::GetEmpty( );
}

FText UK2Node_EnchantedWrapper::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return WrappedNode ? WrappedNode->GetNodeTitle(TitleType) : FText::GetEmpty( );
}

FString UK2Node_EnchantedWrapper::GetDescriptiveCompiledName( ) const
{
	return WrappedNode ? WrappedNode->GetDescriptiveCompiledName( ) : FString( );
}

bool UK2Node_EnchantedWrapper::HasDeprecatedReference( ) const
{
	return WrappedNode ? WrappedNode->HasDeprecatedReference( ) : false;
}

FEdGraphNodeDeprecationResponse UK2Node_EnchantedWrapper::GetDeprecationResponse
(EEdGraphNodeDeprecationType DeprecationType) const
{
	return WrappedNode ? WrappedNode->GetDeprecationResponse(DeprecationType) : FEdGraphNodeDeprecationResponse( );
}

void UK2Node_EnchantedWrapper::PostPlacedNewNode( )
{
	if (WrappedNode) WrappedNode->PostPlacedNewNode( );
}

FString UK2Node_EnchantedWrapper::GetDocumentationLink( ) const
{
	return WrappedNode ? WrappedNode->GetDocumentationLink( ) : FString( );
}

FString UK2Node_EnchantedWrapper::GetDocumentationExcerptName( ) const
{
	return WrappedNode ? WrappedNode->GetDocumentationExcerptName( ) : FString( );
}

FSlateIcon UK2Node_EnchantedWrapper::GetIconAndTint(FLinearColor & OutColor) const
{
	return WrappedNode ? WrappedNode->GetIconAndTint(OutColor) : FSlateIcon( );
}

bool UK2Node_EnchantedWrapper::CanPasteHere(const UEdGraph * TargetGraph) const
{
	return WrappedNode ? WrappedNode->CanPasteHere(TargetGraph) : false;
}

void UK2Node_EnchantedWrapper::PinDefaultValueChanged(UEdGraphPin * Pin)
{
	// if (WrappedNode) WrappedNode->PinDefaultValueChanged(RemapPin(Pin));
}

void UK2Node_EnchantedWrapper::AddSearchMetaDataInfo(TArray<FSearchTagDataPair> & OutTaggedMetaData) const
{
	if (WrappedNode) WrappedNode->AddSearchMetaDataInfo(OutTaggedMetaData);
}

TSharedPtr<SWidget> UK2Node_EnchantedWrapper::CreateNodeImage( ) const
{
	return WrappedNode ? WrappedNode->CreateNodeImage( ) : nullptr;
}

UObject * UK2Node_EnchantedWrapper::GetJumpTargetForDoubleClick( ) const
{
	return WrappedNode ? WrappedNode->GetJumpTargetForDoubleClick( ) : nullptr;
}

bool UK2Node_EnchantedWrapper::CanJumpToDefinition( ) const
{
	return WrappedNode ? WrappedNode->CanJumpToDefinition( ) : false;
}

void UK2Node_EnchantedWrapper::JumpToDefinition( ) const
{
	if (WrappedNode) WrappedNode->JumpToDefinition( );
}

void UK2Node_EnchantedWrapper::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin *> & OldPins)
{
	if (WrappedNode)
	{
		TArray<UEdGraphPin*> RemapedPins;
		RemapedPins.Reserve(OldPins.Num());

		for (const UEdGraphPin * OldPin : OldPins)
		{
			RemapedPins.Push(RemapPin(OldPin));
		}

		WrappedNode->ReallocatePinsDuringReconstruction(RemapedPins);
		ImportPins( );
	}
}

bool UK2Node_EnchantedWrapper::IsNodePure( ) const
{
	return WrappedNode ? WrappedNode->IsNodePure( ) : true;
}

bool UK2Node_EnchantedWrapper::HasExternalDependencies(TArray<UStruct *> * OptionalOutput) const
{
	return WrappedNode ? WrappedNode->HasExternalDependencies(OptionalOutput) : false;
}

void UK2Node_EnchantedWrapper::PostReconstructNode( )
{
	if (WrappedNode) WrappedNode->PostReconstructNode( );
}

bool UK2Node_EnchantedWrapper::ShouldDrawCompact( ) const
{
	return WrappedNode ? WrappedNode->ShouldDrawCompact( ) : false;
}

bool UK2Node_EnchantedWrapper::ShouldDrawAsBead( ) const
{
	return WrappedNode ? WrappedNode->ShouldDrawAsBead( ) : false;
}

FText UK2Node_EnchantedWrapper::GetCompactNodeTitle( ) const
{
	return WrappedNode ? WrappedNode->GetCompactNodeTitle( ) : FText::GetEmpty( );
}

void UK2Node_EnchantedWrapper::PostPasteNode( )
{
	if (WrappedNode) WrappedNode->PostPasteNode( );
}

void UK2Node_EnchantedWrapper::ValidateNodeDuringCompilation(FCompilerResultsLog & MessageLog) const
{
	if (WrappedNode) WrappedNode->ValidateNodeDuringCompilation(MessageLog);
}

bool UK2Node_EnchantedWrapper::ShouldShowNodeProperties( ) const
{
	return WrappedNode ? WrappedNode->ShouldShowNodeProperties( ) : false;
}

void UK2Node_EnchantedWrapper::GetRedirectPinNames(const UEdGraphPin & Pin, TArray<FString> & RedirectPinNames) const
{
	class UK2Node_Exposed : public UK2Node
	{
	public:
		using UK2Node::GetRedirectPinNames;
	};

	if (WrappedNode) ((UK2Node_Exposed *)WrappedNode)->GetRedirectPinNames(RemapPin(Pin), RedirectPinNames);
}

void UK2Node_EnchantedWrapper::NotifyPinConnectionListChanged(UEdGraphPin * Pin)
{
	// if (WrappedNode) WrappedNode->NotifyPinConnectionListChanged(RemapPin(Pin));
}

// TODO Remove
FNodeHandlingFunctor * UK2Node_EnchantedWrapper::CreateNodeHandler(FKismetCompilerContext & CompilerContext) const
{
	return WrappedNode ? WrappedNode->CreateNodeHandler(CompilerContext) : nullptr;
}

// TODO Modify
void UK2Node_EnchantedWrapper::ExpandNode(FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph)
{
	if (WrappedNode) WrappedNode->ExpandNode(CompilerContext, SourceGraph);
}

FName UK2Node_EnchantedWrapper::GetCornerIcon( ) const
{
	return WrappedNode ? WrappedNode->GetCornerIcon( ) : NAME_None;
}

FText UK2Node_EnchantedWrapper::GetToolTipHeading( ) const
{
	return WrappedNode ? WrappedNode->GetToolTipHeading( ) : FText::GetEmpty( );
}

void UK2Node_EnchantedWrapper::GetNodeAttributes(TArray<TKeyValuePair<FString, FString>> & OutNodeAttributes) const
{
	if (WrappedNode) WrappedNode->GetNodeAttributes(OutNodeAttributes);
}

FText UK2Node_EnchantedWrapper::GetMenuCategory( ) const
{
	return WrappedNode ? WrappedNode->GetMenuCategory( ) : FText::GetEmpty( );
}

bool UK2Node_EnchantedWrapper::IsActionFilteredOut(FBlueprintActionFilter const & Filter)
{
	return WrappedNode ? WrappedNode->IsActionFilteredOut(Filter) : true;
}

bool UK2Node_EnchantedWrapper::IsConnectionDisallowed
(const UEdGraphPin * MyPin, const UEdGraphPin * OtherPin, FString & OutReason) const
{
	return WrappedNode ? WrappedNode->IsConnectionDisallowed(RemapPin(MyPin), OtherPin, OutReason) : true;
}

void UK2Node_EnchantedWrapper::ImportPins( )
{
	assumeChecked(WrappedNode);

	// Remove old pins
	for (UEdGraphPin * Pin : Pins)
	{
		RemovePin(Pin);
	}

	// Copy new ones
	for (const UEdGraphPin * Pin : WrappedNode->Pins)
	{
		CreatePin(
			Pin->Direction,
			Pin->PinType.PinCategory,
			Pin->PinType.PinSubCategory,
			Pin->PinType.PinSubCategoryObject.Get( ),
			Pin->PinName,
			FCreatePinParams(Pin->PinType)
		);
	}
}

UEdGraphPin & UK2Node_EnchantedWrapper::RemapPin(const UEdGraphPin & Pin) const
{
	assumeChecked(WrappedNode)
	UEdGraphPin * RemapedPin = WrappedNode->FindPin(Pin.PinName,Pin.Direction);
	check(RemapedPin);
	return *RemapedPin;
}

UEdGraphPin* UK2Node_EnchantedWrapper::RemapPin(const UEdGraphPin* Pin) const
{
	return Pin ? nullptr : &RemapPin(*Pin);
}