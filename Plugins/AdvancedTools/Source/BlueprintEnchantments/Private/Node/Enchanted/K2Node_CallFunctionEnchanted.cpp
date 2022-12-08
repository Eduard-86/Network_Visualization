#include "K2Node_CallFunctionEnchanted.h"

#include "AssumeChecked.h"
#include "BlueprintEditor.h"
#include "K2Node_IfThenElse.h"
#include "KismetCompiler.h"
#include "BlueprintGraph/Classes/EdGraphSchema_K2.h"
#include "Graph/GraphEnchantments.h"

#define LOCTEXT_NAMESPACE "BlueprintEnchantments"

bool UK2Node_CallFunctionEnchanted::CanEnchant(
	const UK2Node & Node
) const
{
	return GetPinToConstructFrom(Node) != nullptr;
}

UK2Node * UK2Node_CallFunctionEnchanted::Enchant(
	const UK2Node & Node
) const
{
	check(HasAllFlags(RF_ClassDefaultObject));
	UK2Node * Result = nullptr;

	if (const auto Pin = GetPinToConstructFrom(Node))
	{
		Result = &DoEnchant(static_cast<const UK2Node_CallFunction &>(Node), *Pin);
	}

	return Result;
}

TOptional<UK2Node *> UK2Node_CallFunctionEnchanted::Enchant(
	UK2Node &           Node,
	const UEdGraphPin & Pin
) const
{
	check(HasAllFlags(RF_ClassDefaultObject));
	TOptional<UK2Node *> Result;
	if (const auto EnchantedCall = Cast<ThisClass>(&Node))
	{
		EnchantedCall->Modify( );
		EnchantedCall->ExpandPin(Pin);
		Result = nullptr;
	}
	else if (const auto FunctionCall = Cast<UK2Node_CallFunction>(&Node))
	{
		if (IsConstructableFrom(*FunctionCall, Pin))
		{
			Result = &DoEnchant(*FunctionCall, Pin);
		}
	}

	return Result;
}

UK2Node & UK2Node_CallFunctionEnchanted::Disenchant( ) const
{
	FGraphNodeCreator<UK2Node_CallFunction> NodeConstructor(*GetGraph( ));

	const auto FunctionCall = NodeConstructor.CreateNode(true);
	assumeChecked(FunctionCall);
	FunctionCall->SetFromFunction(GetTargetFunction( ));
	NodeConstructor.Finalize( );
	FGraphEnchantments::SynchronizeSplittedStructPins(*this, *FunctionCall);

	return *FunctionCall;
}

UK2Node * UK2Node_CallFunctionEnchanted::SplitNextPin( )
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

void UK2Node_CallFunctionEnchanted::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );
	DoExpandPin( );
}

bool UK2Node_CallFunctionEnchanted::IsConnectionDisallowed(
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

void UK2Node_CallFunctionEnchanted::ExpandNode(
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

	// Create function call
	UK2Node_CallFunction & CallFunctionNode = *CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this);
	CallFunctionNode.SetFromFunction(GetTargetFunction( ));
	CallFunctionNode.AllocateDefaultPins( );
	FGraphEnchantments::CopyWildcardTypes(*this, CallFunctionNode);
	UEdGraphPin & OriginalPin = *CallFunctionNode.FindPinChecked(ExpandedPinName, EGPD_Output);

	// Create branch node, move my branches to it
	const UK2Node_IfThenElse & BranchNode = FGraphEnchantments::BeginBranchConstruction(
		CompilerContext,
		*this,
		OriginalPin
	);

	// Connect execution pins
	UEdGraphPin & MyExecutionPin     = *FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
	UEdGraphPin & BranchExecutionPin = *BranchNode.FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
	if (bIsPureFunc)
	{
		CompilerContext.MovePinLinksToIntermediate(MyExecutionPin, BranchExecutionPin);
	}
	else
	{
		UEdGraphPin & OriginalExecutionPin = *CallFunctionNode.FindPinChecked(
			UEdGraphSchema_K2::PN_Execute,
			EGPD_Input
		);
		UEdGraphPin & OriginalThenPin = *CallFunctionNode.FindPinChecked(
			UEdGraphSchema_K2::PN_Then,
			EGPD_Output
		);

		CompilerContext.MovePinLinksToIntermediate(MyExecutionPin, OriginalExecutionPin);
		OriginalThenPin.MakeLinkTo(&BranchExecutionPin);
	}

	FGraphEnchantments::TransferPinLinks(CompilerContext, *this, CallFunctionNode);
}

void UK2Node_CallFunctionEnchanted::ValidateNodeDuringCompilation(FCompilerResultsLog & MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	if (IsInBadState( ))
	{
		MessageLog.Warning(TEXT("Couldn't split pin into Execs!"));
	}
}

void UK2Node_CallFunctionEnchanted::GetPinHoverText(
	const UEdGraphPin & Pin,
	FString &           HoverTextOut
) const
{
	if (UNLIKELY(!bPinTooltipsValid && !IsInBadState()))
	{
		// Get branch pins
		UEdGraphPin & TrueBranchPin = *FindPinChecked(
			FGraphEnchantments::MakeTrueBranchName(ExpandedPinName),
			EGPD_Output
		);
		UEdGraphPin & FalseBranchPin = *FindPinChecked(
			FGraphEnchantments::MakeFalseBranchName(ExpandedPinName),
			EGPD_Output
		);

		// A maneuver to acquire correct description
		FName TrueBranchName  = TrueBranchPin.PinName;
		TrueBranchPin.PinName = ExpandedPinName;

		// Let it refresh all descriptions
		Super::GetPinHoverText(Pin, HoverTextOut);

		// Finalize the maneuver, copy description to false branch
		TrueBranchPin.PinName     = TrueBranchName;
		FalseBranchPin.PinToolTip = TrueBranchPin.PinToolTip;

		// Don't to it again
		bPinTooltipsValid = true;
	}

	return UK2Node::GetPinHoverText(Pin, HoverTextOut);
}

ERenamePinResult UK2Node_CallFunctionEnchanted::RenameUserDefinedPinImpl(
	const FName OldName,
	const FName NewName,
	const bool  bTest
)
{
	return FGraphEnchantments::HandlePinRenaming(*this, ExpandedPinName, OldName, NewName, bTest);
}

UK2Node_CallFunctionEnchanted & UK2Node_CallFunctionEnchanted::DoEnchant(
	const UK2Node_CallFunction & Node,
	const UEdGraphPin &          Pin
)
{
	FGraphNodeCreator<UK2Node_CallFunctionEnchanted> NodeConstructor(*Node.GetGraph( ));

	const auto Enchantment = NodeConstructor.CreateNode(true);
	assumeChecked(Enchantment);
	Enchantment->SetFromFunction(Node.GetTargetFunction( ));
	// Do not call public method to shorten initialization and avoid excess reconstruction
	Enchantment->ExpandedPinName = Pin.PinName;
	NodeConstructor.Finalize( );
	FGraphEnchantments::SynchronizeSplittedStructPins(Node, *Enchantment);

	return *Enchantment;
}

bool UK2Node_CallFunctionEnchanted::IsConstructableFrom(
	const UK2Node_CallFunction & Node,
	const UEdGraphPin &          Pin
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

bool UK2Node_CallFunctionEnchanted::IsConstructableFrom(
	const UK2Node_CallFunction & Node
)
{
	return GetPinToConstructFrom(Node) != nullptr;
}

UEdGraphPin * UK2Node_CallFunctionEnchanted::GetPinToConstructFrom(
	const UK2Node & Node
)
{
	UEdGraphPin * Result = nullptr;

	if (const auto FunctionCall = Cast<Super>(&Node))
	{
		Result = GetPinToConstructFrom(*FunctionCall);
	}

	return Result;
}

UEdGraphPin * UK2Node_CallFunctionEnchanted::GetPinToConstructFrom(
	const UK2Node_CallFunction & Node
)
{
	int           NumExecPins        = 0;
	int           NumThenPins        = 0;
	UEdGraphPin * PinToConstructFrom = nullptr;

	for (int i = Node.Pins.Num( ) - 1; i >= 0; --i)
	{
		UEdGraphPin * Pin = Node.Pins[i];
		assumeChecked(Pin);

		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
		{
			(Pin->Direction == EGPD_Input ? NumExecPins : NumThenPins)++;
		}

		if (FGraphEnchantments::IsPinSplittable(*Pin))
			PinToConstructFrom = Pin;
	}

	if (!((NumExecPins == 0 && NumThenPins == 0) || (NumExecPins == 1 && NumThenPins == 1)))
		PinToConstructFrom = nullptr;

	return PinToConstructFrom;
}


void UK2Node_CallFunctionEnchanted::ExpandPin(
	const FName PinName
)
{
	ExpandedPinName = PinName;
	GetSchema( )->ReconstructNode(*this);
}

void UK2Node_CallFunctionEnchanted::ExpandPin(
	const UEdGraphPin & Pin
)
{
	ExpandPin(Pin.PinName);
}

void UK2Node_CallFunctionEnchanted::DoExpandPin( )
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

	const UEdGraphPin * OriginalExecPin = FindPin(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
	UEdGraphPin *       OriginalThenPin = FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output);

	if (bIsPureFunc)
	{
		if (OriginalThenPin || OriginalExecPin)
			return;

		FCreatePinParams Params;
		Params.Index = 0;
		CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute, Params);
	}
	else
	{
		RemovePin(OriginalThenPin);
	}

	FGraphEnchantments::ExpandPin(*this, *ExpandedPin);
}

bool UK2Node_CallFunctionEnchanted::IsInBadState( ) const
{
	return GetDesiredEnabledState( ) == ENodeEnabledState::Disabled;
}

void UK2Node_CallFunctionEnchanted::SetBadState( )
{
	SetEnabledState(ENodeEnabledState::Disabled, false);
}

void UK2Node_CallFunctionEnchanted::ResetBadState( )
{
	SetEnabledState(ENodeEnabledState::Enabled, false);
}

#undef LOCTEXT_NAMESPACE
