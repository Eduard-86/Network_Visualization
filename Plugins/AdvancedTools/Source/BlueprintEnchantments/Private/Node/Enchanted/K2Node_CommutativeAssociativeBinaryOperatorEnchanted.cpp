#include "K2Node_CommutativeAssociativeBinaryOperatorEnchanted.h"

#include "AssumeChecked.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_IfThenElse.h"
#include "KismetCompiler.h"
#include "ScopedTransaction.h"
#include "Graph/GraphEnchantments.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Util/VTable.h"

#define LOCTEXT_NAMESPACE "BlueprintEnchantments"

bool UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::CanEnchant(
	const UK2Node & Node
) const
{
	return Cast<Super>(&Node) != nullptr;
}

UK2Node * UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::Enchant(
	const UK2Node & Node
) const
{
	check(HasAllFlags(RF_ClassDefaultObject));
	UK2Node * Result = nullptr;

	if (const auto Operator = Cast<UK2Node_CommutativeAssociativeBinaryOperator>(&Node))
	{
		Result = &DoEnchant(*Operator);
	}

	return Result;
}

TOptional<UK2Node *> UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::Enchant(
	UK2Node &           Node,
	const UEdGraphPin & Pin
) const
{
	check(HasAllFlags(RF_ClassDefaultObject));
	TOptional<UK2Node *> Result;

	if (!Node.IsA<ThisClass>( ))
	{
		if (const auto Operator = Cast<UK2Node_CommutativeAssociativeBinaryOperator>(&Node))
		{
			check(&Pin == Operator->GetReturnValuePin());
			Result = &DoEnchant(*Operator);
		}
	}

	return Result;
}

UK2Node & UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::Disenchant( ) const
{
	FGraphNodeCreator<UK2Node_CommutativeAssociativeBinaryOperator> NodeConstructor(*GetGraph( ));

	const auto Operator = NodeConstructor.CreateNode(true);
	assumeChecked(Operator);
	Operator->SetFromFunction(GetTargetFunction( ));
	NodeConstructor.Finalize( );
	AddMissingPins(*this, *Operator);

	return *Operator;
}

UK2Node * UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::SplitNextPin( )
{
	return &Disenchant( );
}

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::AllocateDefaultPins( )
{
	FVTable__CallSuper(AllocateDefaultPins);
	DoExpandPin( );
}

bool UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::IsConnectionDisallowed(
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

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::ReallocatePinsDuringReconstruction(
	TArray<UEdGraphPin *> & OldPins
)
{
	FVTable__CallSuper(ReallocatePinsDuringReconstruction, OldPins);
	DoExpandPin( );
}

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::ExpandNode(
	FKismetCompilerContext & CompilerContext,
	UEdGraph *               SourceGraph
)
{
	if (IsInBadState( ))
		return;

	// If construction went wrong, then act the usual way
	if (FGraphEnchantments::IsOriginalPinPresent(*this, UEdGraphSchema_K2::PN_ReturnValue))
	{
		FVTable__CallSuper(ExpandNode, CompilerContext, SourceGraph);
		return;
	}

	// Create operator node
	auto & OperatorNode = *CompilerContext.SpawnIntermediateNode<UK2Node_CommutativeAssociativeBinaryOperator>(this);
	OperatorNode.SetFromFunction(GetTargetFunction( ));
	OperatorNode.AllocateDefaultPins( );

	// Get it's return pin
	UEdGraphPin * OperatorResultPin = nullptr;
	for (UEdGraphPin * Pin : OperatorNode.Pins)
	{
		if (Pin->Direction == EGPD_Output)
		{
			OperatorResultPin = Pin;
			break;
		}
	}
	check(OperatorResultPin);

	// Synchronize pin count
	AddMissingPins(*this, OperatorNode);

	//
	FGraphEnchantments::FinalizeNodeExpansion(CompilerContext, *this, OperatorNode, *OperatorResultPin);
}

// Exact copy
void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::AddInputPin( )
{
	if (CanAddPin( ))
	{
		FScopedTransaction Transaction(NSLOCTEXT("CommutativeAssociativeBinaryOperatorNode", "AddPinTx", "AddPin"));
		Modify( );

		AddInputPinInner(NumAdditionalInputs);
		++NumAdditionalInputs;

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint( ));
	}
}

// Not actually sure if all these checks are needed if node is simply expanded and pruned away
// Let it be just in case
void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::ValidateNodeDuringCompilation(
	FCompilerResultsLog & MessageLog
) const
{
	Super::Super::ValidateNodeDuringCompilation(MessageLog);

	if (IsInBadState( ))
	{
		MessageLog.Warning(TEXT("Couldn't split pin into Execs!"));
	}

	if (const UFunction * Function = GetTargetFunction( ))
	{
		if (Function->HasAnyFunctionFlags(FUNC_BlueprintPure))
		{
			UEdGraphPin *         SelfPin            = FindSelfPin( );
			const FEdGraphPinType InputType          = GetType( );
			int32                 OutPinCount        = 0;
			int32                 InPinCount         = 0;
			bool                  bConsistentPinType = true;

			for (UEdGraphPin * Pin : Pins)
			{
				if (Pin->Direction == EEdGraphPinDirection::EGPD_Output)
				{
					++OutPinCount;
				}
				else if (Pin != SelfPin)
				{
					if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
					{
						++InPinCount;
					}
					else if (Pin->PinType != InputType)
					{
						bConsistentPinType = false;
					}
				}
			}

			// @formatter:wrap_lines false
			if (OutPinCount != 2)
			{
				MessageLog.Error(*LOCTEXT("InvalidOutPin", "@@ - Enchanted commutative associative binary operators must have 2 output execution pins.").ToString( ), this);
			}
			if (InPinCount != 1)
			{
				MessageLog.Error(*LOCTEXT("InvalidInPin", "@@ - Enchanted commutative associative binary operators must have a single input execution pin.").ToString( ), this);
			}
			if (!bConsistentPinType)
			{
				MessageLog.Error(*NSLOCTEXT("CommutativeAssociativeBinaryOperatorNode", "InconsistentType", "@@ - Commutative associative binary operator inputs must all be of the same type.").ToString( ), this);
			}
		}
		else
		{
			MessageLog.Error(*NSLOCTEXT("CommutativeAssociativeBinaryOperatorNode", "MustBePure", "@@ - Commutative associative binary operators must be pure functions.").ToString( ), this);
		}
		// @formatter:wrap_lines restore
	}
}

// Compile-time linking failure fix
void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::GetNodeContextMenuActions(
	UToolMenu *                    Menu,
	UGraphNodeContextMenuContext * Context
) const
{
	FVTable__CallSuper(GetNodeContextMenuActions, Menu, Context);
}

// Compile-time linking failure fix
bool UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::CanAddPin( ) const
{
	return FVTable__CallSuperInterface(CanAddPin, IK2Node_AddPinInterface);
}

#if ENGINE_MAJOR_VERSION >= 5
void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::RemoveInputPin(UEdGraphPin * Pin)
{
	return FVTable__CallSuperInterface(RemoveInputPin, IK2Node_AddPinInterface, Pin);
}

bool UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::CanRemovePin(const UEdGraphPin * Pin) const
{
	return FVTable__CallSuperInterface(CanRemovePin, IK2Node_AddPinInterface, Pin);
}
#endif

// Modification
FEdGraphPinType UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::GetType( ) const
{
	for (const UEdGraphPin * Pin : Pins)
	{
		if (
			Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec
			&& Pin->PinName != UEdGraphSchema_K2::PN_Self)
		{
			return Pin->PinType;
		}
	}

	return FEdGraphPinType( );
}

// Exact copy
void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::AddInputPinInner(
	const int32 AdditionalPinIndex
)
{
	const FEdGraphPinType          InputType = GetType( );
	UEdGraphNode::FCreatePinParams PinParams;
	PinParams.ContainerType     = InputType.ContainerType;
	PinParams.bIsReference      = InputType.bIsReference;
	PinParams.ValueTerminalType = InputType.PinValueType;
	CreatePin(
		EGPD_Input,
		InputType.PinCategory,
		InputType.PinSubCategory,
		InputType.PinSubCategoryObject.Get( ),
		GetNameForPin(AdditionalPinIndex + BinaryOperatorInputsNum),
		PinParams
	);
}

// Exact copy
FName UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::GetNameForPin(
	const int32 PinIndex
)
{
	check(PinIndex < GetMaxInputPinsNum());
	const FName Name(*FString::Chr(TCHAR('A') + PinIndex));
	return Name;
}

// Exact copy
int32 UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::GetMaxInputPinsNum( )
{
	return (TCHAR('Z') - TCHAR('A'));
}

UK2Node_CommutativeAssociativeBinaryOperatorEnchanted &
UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::DoEnchant(
	const UK2Node_CommutativeAssociativeBinaryOperator & Node
)
{
	FGraphNodeCreator<UK2Node_CommutativeAssociativeBinaryOperatorEnchanted> NodeConstructor(*Node.GetGraph( ));

	const auto Enchantment = NodeConstructor.CreateNode(true);
	assumeChecked(Enchantment);
	Enchantment->SetFromFunction(Node.GetTargetFunction( ));
	NodeConstructor.Finalize( );
	AddMissingPins(Node, *Enchantment);

	return *Enchantment;
}

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::AddMissingPins(
	const UK2Node_CommutativeAssociativeBinaryOperator &    Sample,
	UK2Node_CommutativeAssociativeBinaryOperatorEnchanted & WhomToAdd
)
{
	// Usual operator has 3 basic pins - 2 in, 1 out
	// Enchanted      has 5 basic pins - 3 in, 2 out
	DoAddMissingPins(WhomToAdd, Sample.Pins.Num( ) + 2 - WhomToAdd.Pins.Num( ));
}

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::AddMissingPins(
	const UK2Node_CommutativeAssociativeBinaryOperatorEnchanted & Sample,
	UK2Node_CommutativeAssociativeBinaryOperator &                WhomToAdd
)
{
	// Check overload for comments
	DoAddMissingPins(WhomToAdd, Sample.Pins.Num( ) - 2 - WhomToAdd.Pins.Num( ));
}

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::DoAddMissingPins(
	IK2Node_AddPinInterface & WhomToAdd,
	const int32               NumPins
)
{
	for (int32 i = 0; i < NumPins; i++)
	{
		WhomToAdd.AddInputPin( );
	}
}

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::DoExpandPin( )
{
	UEdGraphPin * ExpandedPin           = FGraphEnchantments::GetOriginalPin(*this, UEdGraphSchema_K2::PN_ReturnValue);
	const auto    AvailableEnchantments = FGraphEnchantments::GetAvailablePinEnchantments(this, ExpandedPin);
	// Can not check for unexpected execution pins I guess - this kind of node only supports pure functions

	if (UNLIKELY(!ExpandedPin || AvailableEnchantments != Expansion))
	{
		SetBadState( );
		FGraphEnchantments::NotifyBranchEmbeddingFailed(*this);
		return;
	}
	ResetBadState( );

	FCreatePinParams Params;
	Params.Index = 0;

	CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Exec,
		UEdGraphSchema_K2::PN_Execute,
		Params
	);

	FGraphEnchantments::ExpandPin(*this, *ExpandedPin);
}

bool UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::IsInBadState( ) const
{
	return GetDesiredEnabledState( ) == ENodeEnabledState::Disabled;
}

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::SetBadState( )
{
	SetEnabledState(ENodeEnabledState::Disabled, false);
}

void UK2Node_CommutativeAssociativeBinaryOperatorEnchanted::ResetBadState( )
{
	SetEnabledState(ENodeEnabledState::Enabled, false);
}

#undef LOCTEXT_NAMESPACE
