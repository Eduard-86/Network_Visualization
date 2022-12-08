#include "K2Node_MakeCompilerMessage.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "KismetCompiler.h"
#include "Kismet2/CompilerResultsLog.h"

#define LOCTEXT_NAMESPACE "UK2Node_MakeCompilerMessage"



namespace PinName
{
const FName Message = "Message";
const FName Type    = "Type";
}



#define EnumToTextChecked(Type) ((void)ECompilerMessageType::Type, TEXT(#Type))

void UK2Node_MakeCompilerMessage::AllocateDefaultPins( )
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PN_Then);

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, PinName::Message)
			->DefaultValue = TEXT("A message from MakeCompilerMessage node");

	auto MessageTypePin                          = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte, PinName::Type);
	MessageTypePin->PinType.PinSubCategoryObject = StaticEnum<ECompilerMessageType>( );
	MessageTypePin->DefaultValue                 = EnumToTextChecked(Warning);
}

FText UK2Node_MakeCompilerMessage::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Make Compiler Message");
}

FText UK2Node_MakeCompilerMessage::GetTooltipText( ) const
{
	return LOCTEXT("NodeToolTip", "Generates a compiler message");
}

void UK2Node_MakeCompilerMessage::PinConnectionListChanged(UEdGraphPin * Pin)
{
	if (Pin->PinName != UEdGraphSchema_K2::PN_Execute && Pin->PinName != UEdGraphSchema_K2::PN_Then)
		return;
	if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard && Pin->LinkedTo.Num( ) > 0)
		return;

	UEdGraphPin * OtherPin = FindPinChecked(
		Pin->PinName == UEdGraphSchema_K2::PN_Execute
		? UEdGraphSchema_K2::PN_Then
		: UEdGraphSchema_K2::PN_Execute
	);
	if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard && Pin->LinkedTo.Num( ) > 0)
	{
		Pin->PinType      = Pin->LinkedTo[0]->PinType;
		OtherPin->PinType = Pin->PinType;
	}
	else if (Pin->LinkedTo.Num( ) == 0 && OtherPin->LinkedTo.Num( ) == 0)
	{
		Pin->PinType.PinCategory      = UEdGraphSchema_K2::PC_Wildcard;
		OtherPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
	}
}

bool UK2Node_MakeCompilerMessage::IsNodePure( ) const
{
	return FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input)->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec;
}

void UK2Node_MakeCompilerMessage::ExpandNode(FKismetCompilerContext & CompilerContext, UEdGraph * SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin & InPin  = *FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
	UEdGraphPin & OutPin = *FindPinChecked(UEdGraphSchema_K2::PN_Then, EGPD_Output);

	for (UEdGraphPin * From : InPin.LinkedTo)
	{
		for (UEdGraphPin * To : OutPin.LinkedTo)
		{
			From->MakeLinkTo(To);
			To->DefaultObject    = InPin.DefaultObject;
			To->DefaultValue     = InPin.DefaultValue;
			To->DefaultTextValue = InPin.DefaultTextValue;
		}
	}

	InPin.BreakAllPinLinks( );
	OutPin.BreakAllPinLinks( );

	const FString & EnumValue = FindPinChecked(PinName::Type, EGPD_Input)->DefaultValue;
	const FString   Message   = FString("@@: ") + FindPinChecked(PinName::Message, EGPD_Input)->DefaultValue;
	if (EnumValue == EnumToTextChecked(Note))
	{
		CompilerContext.MessageLog.Note(*Message, this);
	}
	else if (EnumValue == EnumToTextChecked(Error))
	{
		CompilerContext.MessageLog.Error(*Message, this);
	}
	else
	{
		CompilerContext.MessageLog.Warning(*Message, this);
	}
}

FSlateIcon UK2Node_MakeCompilerMessage::GetIconAndTint(FLinearColor & OutColor) const
{
	OutColor = FLinearColor::FromSRGBColor(FColor(255, 255, 0));
	return FSlateIcon("EditorStyle", "Icons.Warning");
}

void UK2Node_MakeCompilerMessage::GetMenuActions(FBlueprintActionDatabaseRegistrar & ActionRegistrar) const
{
	UClass * ActionKey = GetClass( );

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner * NodeSpawner = UBlueprintNodeSpawner::Create(GetClass( ));
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_MakeCompilerMessage::GetMenuCategory( ) const
{
	return LOCTEXT("Category", "Development");
}

#undef LOCTEXT_NAMESPACE
#undef EnumToTextChecked
