#include "K2Node_MacroInstanceEnchanted.h"

#include "AssumeChecked.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_IfThenElse.h"
#include "KismetCompiler.h"
#include "Graph/GraphEnchantments.h"
#include "Util/VTable.h"

bool UK2Node_MacroInstanceEnchanted::CanEnchant(const UK2Node & Node) const
{
	return GetPinToConstructFrom(Node) != nullptr;
}

UK2Node * UK2Node_MacroInstanceEnchanted::Enchant(
	const UK2Node & Node
) const
{
	check(HasAllFlags(RF_ClassDefaultObject));
	UK2Node * Result = nullptr;

	if (const auto Pin = GetPinToConstructFrom(Node))
	{
		Result = &DoEnchant(static_cast<const Super &>(Node), *Pin);
	}

	return Result;
}

TOptional<UK2Node *> UK2Node_MacroInstanceEnchanted::Enchant(
	UK2Node &           Node,
	const UEdGraphPin & Pin
) const
{
	check(HasAllFlags(RF_ClassDefaultObject));
	TOptional<UK2Node *> Result;

	if (const auto EnchantedMacro = Cast<ThisClass>(&Node))
	{
		EnchantedMacro->Modify( );
		EnchantedMacro->ExpandPin(Pin);
		Result = nullptr;
	}
	else if (const auto Macro = Cast<Super>(&Node))
	{
		if (IsConstructableFrom(*Macro, Pin))
		{
			Result = &DoEnchant(*Macro, Pin);
		}
	}

	return Result;
}

UK2Node & UK2Node_MacroInstanceEnchanted::Disenchant( ) const
{
	FGraphNodeCreator<UK2Node_MacroInstance> NodeConstructor(*GetGraph( ));

	const auto MacroNode = NodeConstructor.CreateNode(true);
	assumeChecked(MacroNode);
	MacroNode->SetMacroGraph(GetMacroGraph( ));
	NodeConstructor.Finalize( );
	FGraphEnchantments::SynchronizeSplittedStructPins(*this, *MacroNode);

	return *MacroNode;
}

UK2Node * UK2Node_MacroInstanceEnchanted::SplitNextPin( )
{
	const UEdGraphPin * NextPin = FGraphEnchantments::GetNextExpandablePin(
		*this,
		NameOfPinAfterExpandedOne
	);
	if (NextPin)
	{
		ExpandPin(*NextPin);
		return nullptr;
	}
	else
	{
		return &Disenchant( );
	}
}

ERenamePinResult UK2Node_MacroInstanceEnchanted::RenameUserDefinedPinImpl(
	const FName OldName,
	const FName NewName,
	const bool  bTest
)
{
	return FGraphEnchantments::HandlePinRenaming(*this, ExpandedPinName, OldName, NewName, bTest);
}

void UK2Node_MacroInstanceEnchanted::AllocateDefaultPins( )
{
	FVTable__CallSuper(AllocateDefaultPins);
	DoExpandPin( );
}

bool UK2Node_MacroInstanceEnchanted::IsConnectionDisallowed(
	const UEdGraphPin * MyPin,
	const UEdGraphPin * OtherPin,
	FString &           OutReason
) const
{
	if (IsInBadState( ))
	{
		OutReason = "Node is in damaged state";
		return true;
	}

	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}

void UK2Node_MacroInstanceEnchanted::ExpandNode(
	FKismetCompilerContext & CompilerContext,
	UEdGraph *               SourceGraph
)
{
	if (IsInBadState( ))
		return;

	// If construction went wrong, then act the usual way
	if (FGraphEnchantments::IsOriginalPinPresent(*this, ExpandedPinName))
	{
		Super::ExpandNode(CompilerContext, SourceGraph);
		return;
	}

	UK2Node_MacroInstance & MacroNode = *CompilerContext.SpawnIntermediateNode<UK2Node_MacroInstance>(this);
	MacroNode.SetMacroGraph(GetMacroGraph( ));
	MacroNode.AllocateDefaultPins( );
	MacroNode.ResolvedWildcardType = ResolvedWildcardType;
	FGraphEnchantments::CopyWildcardTypes(*this, MacroNode);
	UEdGraphPin & OriginalPin = *MacroNode.FindPinChecked(ExpandedPinName, EGPD_Output);

	// Create branch node, move my branches to it
	const UK2Node_IfThenElse & BranchNode = FGraphEnchantments::BeginBranchConstruction(
		CompilerContext,
		*this,
		OriginalPin
	);

	// Connect execution pins
	UEdGraphPin & MyExecutionPin     = *FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
	UEdGraphPin & BranchExecutionPin = *BranchNode.FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input);

	if (UEdGraphPin * OriginalExecutionPin = FindExecutionPin(MacroNode))
	{
		UEdGraphPin * OriginalThenPin = FindThenPin(MacroNode);
		check(OriginalThenPin);

		CompilerContext.MovePinLinksToIntermediate(MyExecutionPin, *OriginalExecutionPin);
		OriginalThenPin->MakeLinkTo(&BranchExecutionPin);
	}
	else
	{
		CompilerContext.MovePinLinksToIntermediate(MyExecutionPin, BranchExecutionPin);
	}

	FGraphEnchantments::TransferPinLinks(CompilerContext, *this, MacroNode);

	// Force compiler perform another round of macro expansion (lucky me)
	class Exposed : public FKismetCompilerContext
	{
	public:
		using FKismetCompilerContext::ExpandTunnelsAndMacros;
	};

	((Exposed &)(CompilerContext)).ExpandTunnelsAndMacros(SourceGraph);
}

void UK2Node_MacroInstanceEnchanted::ValidateNodeDuringCompilation(FCompilerResultsLog & MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	if (IsInBadState( ))
	{
		MessageLog.Warning(TEXT("Couldn't split pin into Execs!"));
	}
}

bool UK2Node_MacroInstanceEnchanted::ShouldMergeChildGraphs( ) const
{
	// Macro nodes are expanded as macros before they are expanded as nodes
	// This allows to override this behaviour
	// FKismetCompilerContext::ExpandTunnelsAndMacros
	// KismetCompiler.cpp::3523
	return false;
}

void UK2Node_MacroInstanceEnchanted::NodeConnectionListChanged( )
{
	// Could just override ReallocatePinsDuringConstruction, but here comes some vtable problems
	// that gonna be fixed later, so let it just be so

	// Resets on Super call, so cache now
	const bool            bIsReconstructing = bReconstructNode;
	TArray<UEdGraphPin *> ExecLinks, TrueLinks, FalseLinks;
	if (bIsReconstructing)
	{
		ExecLinks  = FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input)->LinkedTo;
		TrueLinks  = FindPinChecked(FGraphEnchantments::MakeTrueBranchName(ExpandedPinName), EGPD_Output)->LinkedTo;
		FalseLinks = FindPinChecked(FGraphEnchantments::MakeFalseBranchName(ExpandedPinName), EGPD_Output)->LinkedTo;
	}

	FVTable__CallSuper(NodeConnectionListChanged);

	if (bIsReconstructing)
	{
		DoExpandPin( );

		UEdGraphPin * ExecPin  = FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
		UEdGraphPin * TruePin  = FindPinChecked(FGraphEnchantments::MakeTrueBranchName(ExpandedPinName), EGPD_Output);
		UEdGraphPin * FalsePin = FindPinChecked(FGraphEnchantments::MakeFalseBranchName(ExpandedPinName), EGPD_Output);

		for (UEdGraphPin * Link : ExecLinks)
		{
			ExecPin->MakeLinkTo(Link);
		}
		for (UEdGraphPin * Link : TrueLinks)
		{
			TruePin->MakeLinkTo(Link);
		}
		for (UEdGraphPin * Link : FalseLinks)
		{
			FalsePin->MakeLinkTo(Link);
		}
	}
}

UK2Node_MacroInstanceEnchanted::ThisClass & UK2Node_MacroInstanceEnchanted::DoEnchant(
	const Super &       Node,
	const UEdGraphPin & Pin
)
{
	FGraphNodeCreator<UK2Node_MacroInstanceEnchanted> NodeConstructor(*Node.GetGraph( ));

	const auto Enchantment = NodeConstructor.CreateNode(true);
	assumeChecked(Enchantment);
	Enchantment->SetMacroGraph(Node.GetMacroGraph( ));
	// Do not call public method to shorten initialization and avoid excess reconstruction
	Enchantment->ExpandedPinName = Pin.PinName;
	NodeConstructor.Finalize( );
	FGraphEnchantments::SynchronizeSplittedStructPins(Node, *Enchantment);

	return *Enchantment;
}

bool UK2Node_MacroInstanceEnchanted::IsConstructableFrom(
	const Super &       Node,
	const UEdGraphPin & Pin
)
{
	bool Result = false;

	if (const auto FirstPinToConstructFrom = GetPinToConstructFrom(Node))
	{
		if (FirstPinToConstructFrom == &Pin)
		{
			Result = true;
		}
		else
		{
			Result = FGraphEnchantments::IsPinSplittable(Pin);
		}
	}

	return Result;
}

bool UK2Node_MacroInstanceEnchanted::IsConstructableFrom(
	const UK2Node_MacroInstance & MacroNode
)
{
	return GetPinToConstructFrom(MacroNode) != nullptr;
}

UEdGraphPin * UK2Node_MacroInstanceEnchanted::GetPinToConstructFrom(
	const UK2Node & Node
)
{
	UEdGraphPin * Result = nullptr;

	if (const auto Macro = Cast<Super>(&Node))
	{
		Result = GetPinToConstructFrom(*Macro);
	}

	return Result;
}

UEdGraphPin * UK2Node_MacroInstanceEnchanted::GetPinToConstructFrom(
	const UK2Node_MacroInstance & Node
)
{
	bool          bHasExplicitExecutionPin = false;
	bool          bHasExplicitThenPin      = false;
	bool          bIsPure                  = false;
	int           NumExecPins              = 0;
	int           NumThenPins              = 0;
	UEdGraphPin * PinToConstructFrom       = nullptr;

	for (int i = Node.Pins.Num( ) - 1; i >= 0; --i)
	{
		UEdGraphPin * Pin = Node.Pins[i];
		assumeChecked(Pin);

		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
		{
			if (!bHasExplicitExecutionPin && Pin->Direction == EGPD_Input)
			{
				if (Pin->PinName == UEdGraphSchema_K2::PN_Execute)
					bHasExplicitExecutionPin = true;
				else
					NumExecPins++;
			}
			else
			{
				assumeChecked(Pin->Direction == EGPD_Output);
				if (!bHasExplicitThenPin && Pin->PinName == UEdGraphSchema_K2::PN_Then)
					bHasExplicitThenPin = true;
				else
					NumThenPins++;
			}
		}

		if (FGraphEnchantments::IsPinSplittable(*Pin))
			PinToConstructFrom = Pin;
	}

	if (!bHasExplicitExecutionPin)
		bHasExplicitExecutionPin = NumExecPins == 1;
	if (!bHasExplicitThenPin)
		bHasExplicitThenPin = NumThenPins == 1;
	bIsPure = NumExecPins == 0 && NumThenPins == 0;

	if (!(bIsPure || (bHasExplicitExecutionPin && bHasExplicitThenPin)))
		PinToConstructFrom = nullptr;

	return PinToConstructFrom;
}

void UK2Node_MacroInstanceEnchanted::ExpandPin(
	const FName PinName
)
{
	ExpandedPinName = PinName;
	GetSchema( )->ReconstructNode(*this);
}

void UK2Node_MacroInstanceEnchanted::ExpandPin(
	const UEdGraphPin & Pin
)
{
	ExpandPin(Pin.PinName);
}

void UK2Node_MacroInstanceEnchanted::DoExpandPin( )
{
	UEdGraphPin *         ExpandedPin      = FGraphEnchantments::GetOriginalPin(*this, ExpandedPinName);
	const EPinEnchantment Enchantment      = FGraphEnchantments::GetAvailablePinEnchantments(this, ExpandedPin);
	const bool            bIsConstructable = IsConstructableFrom(*this);

	if (UNLIKELY(!ExpandedPin || Enchantment != Expansion || !bIsConstructable))
	{
		SetBadState( );
		FGraphEnchantments::NotifyBranchEmbeddingFailed(*this);
		return;
	}
	ResetBadState( );

	NameOfPinAfterExpandedOne = FGraphEnchantments::GetNextPinName(*this, ExpandedPinName);

	UEdGraphPin * OriginalExecPin = FindExecutionPin(*this);
	UEdGraphPin * OriginalThenPin = FindThenPin(*this);

	if (OriginalExecPin && OriginalThenPin)
	{
		RemovePin(OriginalThenPin);
	}
	else
	{
		FCreatePinParams Params;
		Params.Index = 0;
		CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute, Params);
	}

	FGraphEnchantments::ExpandPin(*this, *ExpandedPin);
}

UEdGraphPin * UK2Node_MacroInstanceEnchanted::FindExecutionPin(
	const UEdGraphNode & Node
)
{
	return DoFindPin(Node, EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
}

UEdGraphPin * UK2Node_MacroInstanceEnchanted::FindThenPin(
	const UEdGraphNode & Node
)
{
	return DoFindPin(Node, EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
}

UEdGraphPin * UK2Node_MacroInstanceEnchanted::DoFindPin(
	const UEdGraphNode &       Node,
	const EEdGraphPinDirection Direction,
	const FName                PinCategory,
	const FName                DesiredName
)
{
	UEdGraphPin * Result = nullptr;

	// Reverse loop to get to the first valid result while searching for desired pin
	for (int i = Node.Pins.Num( ) - 1; i >= 0; i--)
	{
		UEdGraphPin & Pin = *Node.Pins[i];
		if (Pin.Direction == Direction && Pin.PinType.PinCategory == PinCategory)
		{
			Result = &Pin;

			// Best candidate
			if (Pin.PinName == DesiredName)
				break;
		}
	}

	return Result;
}

bool UK2Node_MacroInstanceEnchanted::IsInBadState( ) const
{
	return GetDesiredEnabledState( ) == ENodeEnabledState::Disabled;
}

void UK2Node_MacroInstanceEnchanted::SetBadState( )
{
	SetEnabledState(ENodeEnabledState::Disabled, false);
}

void UK2Node_MacroInstanceEnchanted::ResetBadState( )
{
	SetEnabledState(ENodeEnabledState::Enabled, false);
}

// Linker errs fixes //
/*************************************************************************************************/

#pragma region Linker err fixes

void UK2Node_MacroInstanceEnchanted::Serialize(FArchive & Ar)
{
	FVTable__CallSuperOverload(SelectOverload(Serialize, FArchive&), Ar);
}

void UK2Node_MacroInstanceEnchanted::PreloadRequiredAssets( )
{
	FVTable__CallSuper(PreloadRequiredAssets);
}

FText UK2Node_MacroInstanceEnchanted::GetTooltipText( ) const
{
	return FVTable__CallSuper(GetTooltipText);
}

FText UK2Node_MacroInstanceEnchanted::GetKeywords( ) const
{
	return FVTable__CallSuper(GetKeywords);
}

void UK2Node_MacroInstanceEnchanted::PostPasteNode( )
{
	FVTable__CallSuper(PostPasteNode);
}

FLinearColor UK2Node_MacroInstanceEnchanted::GetNodeTitleColor( ) const
{
	return FVTable__CallSuper(GetNodeTitleColor);
}

FText UK2Node_MacroInstanceEnchanted::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FVTable__CallSuper(GetNodeTitle, TitleType);
}

void UK2Node_MacroInstanceEnchanted::GetNodeContextMenuActions(
	UToolMenu *                    Menu,
	UGraphNodeContextMenuContext * Context
) const
{
	return FVTable__CallSuper(GetNodeContextMenuActions, Menu, Context);
}

FString UK2Node_MacroInstanceEnchanted::GetDocumentationLink( ) const
{
	return FVTable__CallSuper(GetDocumentationLink);
}

FString UK2Node_MacroInstanceEnchanted::GetDocumentationExcerptName( ) const
{
	return FVTable__CallSuper(GetDocumentationExcerptName);
}

FSlateIcon UK2Node_MacroInstanceEnchanted::GetIconAndTint(FLinearColor & OutColor) const
{
	return FVTable__CallSuper(GetIconAndTint, OutColor);
}

bool UK2Node_MacroInstanceEnchanted::CanPasteHere(const UEdGraph * TargetGraph) const
{
	return FVTable__CallSuper(CanPasteHere, TargetGraph);
}

void UK2Node_MacroInstanceEnchanted::NotifyPinConnectionListChanged(UEdGraphPin * Pin)
{
	FVTable__CallSuper(NotifyPinConnectionListChanged, Pin);
}

void UK2Node_MacroInstanceEnchanted::PostReconstructNode( )
{
	FVTable__CallSuper(PostReconstructNode);
}

bool UK2Node_MacroInstanceEnchanted::HasExternalDependencies(TArray<UStruct *> * OptionalOutput) const
{
	return FVTable__CallSuper(HasExternalDependencies, OptionalOutput);
}

void UK2Node_MacroInstanceEnchanted::GetNodeAttributes(
	TArray<TKeyValuePair<FString, FString>> & OutNodeAttributes
) const
{
	FVTable__CallSuper(GetNodeAttributes, OutNodeAttributes);
}

FText UK2Node_MacroInstanceEnchanted::GetMenuCategory( ) const
{
	return FVTable__CallSuper(GetMenuCategory);
}

FBlueprintNodeSignature UK2Node_MacroInstanceEnchanted::GetSignature( ) const
{
	return FVTable__CallSuper(GetSignature);
}

bool UK2Node_MacroInstanceEnchanted::IsActionFilteredOut(FBlueprintActionFilter const & Filter)
{
	return FVTable__CallSuper(IsActionFilteredOut, Filter);
}

FText UK2Node_MacroInstanceEnchanted::GetCompactNodeTitle( ) const
{
	return FVTable__CallSuper(GetCompactNodeTitle);
}

bool UK2Node_MacroInstanceEnchanted::ShouldDrawCompact( ) const
{
	return FVTable__CallSuper(ShouldDrawCompact);
}

FName UK2Node_MacroInstanceEnchanted::GetCornerIcon( ) const
{
	return FVTable__CallSuper(GetCornerIcon);
}

void UK2Node_MacroInstanceEnchanted::PostFixupAllWildcardPins(bool bInAllWildcardPinsUnlinked)
{
	FVTable__CallSuper(PostFixupAllWildcardPins, bInAllWildcardPinsUnlinked);
}

#pragma endregion ~ Linker errs fixes
