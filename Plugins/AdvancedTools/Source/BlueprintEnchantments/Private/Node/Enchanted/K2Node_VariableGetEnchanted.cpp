#include "K2Node_VariableGetEnchanted.h"

#include "AssumeChecked.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_IfThenElse.h"
#include "KismetCompiler.h"
#include "Graph/GraphEnchantments.h"

bool UK2Node_VariableGetEnchanted::CanEnchant(
	const UK2Node & Node
) const
{
	return Cast<Super>(&Node) != nullptr;
}

UK2Node * UK2Node_VariableGetEnchanted::Enchant(
	const UK2Node & Node
) const
{
	check(HasAllFlags(RF_ClassDefaultObject));
	UK2Node * Result = nullptr;

	if (const auto Getter = Cast<Super>(&Node))
	{
		Result = &DoEnchant(*Getter);
	}

	return Result;
}

TOptional<UK2Node *> UK2Node_VariableGetEnchanted::Enchant(
	UK2Node &           Node,
	const UEdGraphPin & Pin
) const
{
	check(HasAllFlags(RF_ClassDefaultObject));
	TOptional<UK2Node *> Result;

	if (!Node.IsA<ThisClass>( ))
	{
		if (const auto Getter = Cast<Super>(&Node))
		{
			check(&Pin == Getter->GetValuePin());
			Result = &DoEnchant(*Getter);
		}
	}

	return Result;
}

UK2Node & UK2Node_VariableGetEnchanted::Disenchant( ) const
{
	FGraphNodeCreator<UK2Node_VariableGet> NodeConstructor(*GetGraph( ));

	auto Result = NodeConstructor.CreateNode(true);
	assumeChecked(Result);
	// NewNode->SetFromProperty() fails
	new(&Result->VariableReference) FMemberReference(VariableReference);
	Result->SelfContextInfo = SelfContextInfo;
	NodeConstructor.Finalize( );

	return *Result;
}

UK2Node * UK2Node_VariableGetEnchanted::SplitNextPin( )
{
	return &Disenchant( );
}

void UK2Node_VariableGetEnchanted::AllocateDefaultPins( )
{
	Super::AllocateDefaultPins( );
	ExpandPin( );
}

void UK2Node_VariableGetEnchanted::ReallocatePinsDuringReconstruction(
	TArray<UEdGraphPin *> & OldPins
)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);
	ExpandPin( );
}

bool UK2Node_VariableGetEnchanted::IsConnectionDisallowed(
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

void UK2Node_VariableGetEnchanted::ExpandNode(
	FKismetCompilerContext & CompilerContext,
	UEdGraph *               SourceGraph
)
{
	if (IsInBadState( ))
		return;

	// If construction went wrong, then act the usual way
	if (FGraphEnchantments::IsOriginalPinPresent(*this, GetVarName( )))
	{
		Super::ExpandNode(CompilerContext, SourceGraph);
		return;
	}

	auto & GetVarNode = *CompilerContext.SpawnIntermediateNode<UK2Node_VariableGet>(this);
	// NewNode->SetFromProperty() fails
	new(&GetVarNode.VariableReference) FMemberReference(this->VariableReference);
	GetVarNode.SelfContextInfo = this->SelfContextInfo;
	GetVarNode.AllocateDefaultPins( );
	UEdGraphPin & OriginalPin = *GetVarNode.FindPinChecked(GetVarNode.GetVarName( ), EGPD_Output);

	FGraphEnchantments::FinalizeNodeExpansion(CompilerContext, *this, GetVarNode, OriginalPin);
}

void UK2Node_VariableGetEnchanted::ValidateNodeDuringCompilation(FCompilerResultsLog & MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	if (IsInBadState( ))
	{
		MessageLog.Warning(TEXT("Couldn't split pin into Execs!"));
	}
}

bool UK2Node_VariableGetEnchanted::IsConstructableFrom(
	const UK2Node_VariableGet & Node
)
{
	const UEdGraphPin * ValuePin = Node.GetValuePin( );
	return ValuePin ? FGraphEnchantments::IsPinSplittable(*ValuePin) : false;
}

UK2Node_VariableGetEnchanted::ThisClass & UK2Node_VariableGetEnchanted::DoEnchant(
	const Super & Node
)
{
	FGraphNodeCreator<UK2Node_VariableGetEnchanted> NodeConstructor(*Node.GetGraph( ));

	const auto Enchantment = NodeConstructor.CreateNode(true);
	assumeChecked(Enchantment);
	// NewNode->SetFromProperty() fails
	new(&Enchantment->VariableReference) FMemberReference(Node.VariableReference);
	Enchantment->SelfContextInfo = Node.SelfContextInfo;
	NodeConstructor.Finalize( );

	return *Enchantment;
}

void UK2Node_VariableGetEnchanted::ExpandPin( )
{
	UEdGraphPin *         OriginalPin      = FGraphEnchantments::GetOriginalPin(*this, GetVarName( ));
	const EPinEnchantment Enchantments     = FGraphEnchantments::GetAvailablePinEnchantments(this, OriginalPin);
	const bool            bIsConstructable = IsConstructableFrom(*this);

	if (UNLIKELY(!OriginalPin || Enchantments != Expansion || !bIsConstructable))
	{
		SetBadState( );
		FGraphEnchantments::NotifyBranchEmbeddingFailed(*this);
		return;
	}
	ResetBadState( );

	// Make execution pin
	FCreatePinParams Params;
	Params.Index = 0;

	CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Exec,
		UEdGraphSchema_K2::PN_Execute,
		Params
	);

	FGraphEnchantments::ExpandPin(*this, *OriginalPin);
}

bool UK2Node_VariableGetEnchanted::IsInBadState( ) const
{
	return GetDesiredEnabledState( ) == ENodeEnabledState::Disabled;
}

void UK2Node_VariableGetEnchanted::SetBadState( )
{
	SetEnabledState(ENodeEnabledState::Disabled, false);
}

void UK2Node_VariableGetEnchanted::ResetBadState( )
{
	SetEnabledState(ENodeEnabledState::Enabled, false);
}
