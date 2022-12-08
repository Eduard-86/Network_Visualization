#include "K2Node_BuildTypeSwitch.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"

#define LOCTEXT_NAMESPACE "UK2Node_BuildTypeSwitch"


namespace PinName
{
const FName Shipping    = "Shipping";
const FName Development = "Development";
const FName Debug       = "Debug";
}


UK2Node_BuildTypeSwitch::UK2Node_BuildTypeSwitch(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{}

void UK2Node_BuildTypeSwitch::AllocateDefaultPins( )
{
	CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Exec,
		UEdGraphSchema_K2::PN_Execute
	);
	CreatePin(
		EGPD_Output,
		UEdGraphSchema_K2::PC_Exec,
		PinName::Shipping
	);
	CreatePin(
		EGPD_Output,
		UEdGraphSchema_K2::PC_Exec,
		PinName::Development
	);
}

FText UK2Node_BuildTypeSwitch::GetNodeTitle(ENodeTitleType::Type Title) const
{
	return LOCTEXT("NodeTitle", "Switch Build Type");
}

FText UK2Node_BuildTypeSwitch::GetTooltipText( ) const
{
	return LOCTEXT("NodeToolTip", "Redirects execution flow depending on build type");
}

void UK2Node_BuildTypeSwitch::ExpandNode(
	FKismetCompilerContext & CompilerContext,
	UEdGraph *               SourceGraph
)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin * ExecPin = GetExecPin( );
	UEdGraphPin * TargetPin;

	// Building plugin in real debug mode is barely possible
	// It's always development, no matter what
	// So there's no debug pin
	// As it's a UK2Node, it's also never compiled in shipping mode
	// BUT we can detect cooking process worse or better
	if (IsRunningCommandlet( ))
	{
		TargetPin = GetThenShippingPin( );
	}
	else
	{
		TargetPin = GetThenDevelopmentPin( );
	}

	if (ExecPin->LinkedTo.Num( ) && TargetPin->LinkedTo.Num( ))
	{
		TargetPin = TargetPin->LinkedTo[0];

		for (UEdGraphPin * InPin : ExecPin->LinkedTo)
		{
			InPin->MakeLinkTo(TargetPin);
		}
	}

	BreakAllNodeLinks( );
}

FSlateIcon UK2Node_BuildTypeSwitch::GetIconAndTint(FLinearColor & OutColor) const
{
	OutColor = FLinearColor::White;
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Switch_16x");
	return Icon;
}

void UK2Node_BuildTypeSwitch::GetMenuActions(FBlueprintActionDatabaseRegistrar & ActionRegistrar) const
{
	UClass * ActionKey = GetClass( );

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner * NodeSpawner = UBlueprintNodeSpawner::Create(GetClass( ));
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BuildTypeSwitch::GetMenuCategory( ) const
{
	// FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::FlowControl);
	return FText::FromString("Utilities|FlowControl");
}

UEdGraphPin * UK2Node_BuildTypeSwitch::GetThenDevelopmentPin( ) const
{
	UEdGraphPin * Pin = FindPinChecked(PinName::Development, EGPD_Output);
	return Pin;
}

UEdGraphPin * UK2Node_BuildTypeSwitch::GetThenShippingPin( ) const
{
	UEdGraphPin * Pin = FindPinChecked(PinName::Shipping, EGPD_Output);
	return Pin;
}

#undef LOCTEXT_NAMESPACE
