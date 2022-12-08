#include "K2Node_UpdateDataTableRow.h"

#include "AdvancedEditorBPLibrary.h"

#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "EditorCategoryUtils.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MakeStruct.h"
#include "KismetCompiler.h"
#include "Engine/DataTable.h"
#include "BlueprintGraph/Public/BlueprintActionDatabaseRegistrar.h"
#include "UnrealEd/Public/Kismet2/CompilerResultsLog.h"
#include "UnrealEd/Public/DataTableEditorUtils.h"
#include "AssumeChecked.h"

#define LOCTEXT_NAMESPACE "K2Node_UpdateDataTableRow"

namespace GetDataTableRowHelper
{
const FName DataTablePinName = "DataTable";
const FName RowNamePinName   = "RowName";
const FName RowDataPinName   = "RowData";
}

UK2Node_UpdateDataTableRow::UK2Node_UpdateDataTableRow(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeTooltip = LOCTEXT(
		"NodeTooltip",
		"Updates a data table row in a DataTable with RowData via it's RowName"
	);
	// SetEnabledState(ENodeEnabledState::DevelopmentOnly, false);
}

void UK2Node_UpdateDataTableRow::AllocateDefaultPins( )
{
	// Add execution pins
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	UEdGraphPin * ThenPin = CreatePin(
		EGPD_Output,
		UEdGraphSchema_K2::PC_Exec,
		UEdGraphSchema_K2::PN_Then
	);

	// Add DataTable pin
	UEdGraphPin * DataTablePin = CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Object,
		UDataTable::StaticClass(),
		GetDataTableRowHelper::DataTablePinName
	);
	SetPinToolTip(
		*DataTablePin,
		LOCTEXT("DataTablePinDescription", "The DataTable you want to update a row in")
	);

	// Row Name pin
	UEdGraphPin * RowNamePin = CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Name,
		GetDataTableRowHelper::RowNamePinName
	);
	SetPinToolTip(
		*RowNamePin,
		LOCTEXT("RowNamePinDescription", "The name of the row to update in the DataTable")
	);

	// Result pin
	UEdGraphPin * RowDataPin = CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Wildcard,
		GetDataTableRowHelper::RowDataPinName
	);
	SetPinToolTip(
		*RowDataPin,
		LOCTEXT("RowDataPinDescription", "Data to put into the Datatable")
	);

	Super::AllocateDefaultPins();
}

FText UK2Node_UpdateDataTableRow::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::MenuTitle)
	{
		return LOCTEXT("ListViewTitle", "Update Data Table Row by Name");
	}
	else if (UEdGraphPin * DataTablePin = GetDataTablePin())
	{
		if (DataTablePin->LinkedTo.Num() > 0)
		{
			return LOCTEXT("NodeTitle_Unknown", "Update Data Table Row");
		}
		else if (DataTablePin->DefaultObject == nullptr)
		{
			return LOCTEXT("NodeTitle_None", "Update Data Table Row NONE");
		}
		else if (CachedNodeTitle.IsOutOfDate(this))
		{
			FFormatNamedArguments Args;
			Args.Add(
				TEXT("DataTableName"),
				FText::FromString(DataTablePin->DefaultObject->GetName())
			);

			FText LocFormat = LOCTEXT("NodeTitle", "Update Data Table Row {DataTableName}");
			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText(FText::Format(LocFormat, Args), this);
		}
	}
	else
	{
		return LOCTEXT("NodeTitle_None", "Get Data Table Row NONE");
	}

	return CachedNodeTitle;
}

// void UK2Node_UpdateDataTableRow::PinDefaultValueChanged(UEdGraphPin * ChangedPin)
// {
// 	if (ChangedPin && ChangedPin->PinName == GetDataTableRowHelper::DataTablePinName)
// 	{
// 		RefreshRowDataPinType();
// 	
// 		UEdGraphPin * RowNamePin = GetRowNamePin();
// 		UDataTable *  DataTable  = Cast<UDataTable>(ChangedPin->DefaultObject);
// 		if (RowNamePin)
// 		{
// 			if (DataTable && (RowNamePin->DefaultValue.IsEmpty() || !DataTable->GetRowMap().
// 			                  Contains(*RowNamePin->DefaultValue)))
// 			{
// 				if (auto Iterator = DataTable->GetRowMap().CreateConstIterator())
// 				{
// 					RowNamePin->DefaultValue = Iterator.Key().ToString();
// 				}
// 			}
// 	
// 			RefreshRowNameOptions();
// 		}
// 	}
// }

FText UK2Node_UpdateDataTableRow::GetTooltipText( ) const
{
	return NodeTooltip;
}

void UK2Node_UpdateDataTableRow::ExpandNode(FKismetCompilerContext & CompilerContext,
                                            UEdGraph *               SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin * OriginalDataTablePin = GetDataTablePin();
	UDataTable *  Table                = (OriginalDataTablePin != NULL)
		                                     ? Cast<UDataTable>(OriginalDataTablePin->DefaultObject)
		                                     : NULL;
	if ((nullptr == OriginalDataTablePin) || (
		    0 == OriginalDataTablePin->LinkedTo.Num() && nullptr == Table))
	{
		CompilerContext.MessageLog.Error(
			*LOCTEXT(
				"UpdateDataTableRowNoDataTable_Error",
				"UpdateDataTableRow must have a DataTable specified."
			).ToString(),
			this
		);
		// we break exec links so this is the only error we get
		BreakAllNodeLinks();
		return;
	}

	// FUNCTION NODE
	const FName FunctionName = GET_FUNCTION_NAME_CHECKED(
		UAdvancedToolsEditorBPLibrary,
		UpdateDataTableRowByName
	);
	UK2Node_CallFunction * UpdateDataTableRowFunction = CompilerContext.SpawnIntermediateNode<
		UK2Node_CallFunction>(
		this,
		SourceGraph
	);
	UpdateDataTableRowFunction->FunctionReference.SetExternalMember(
		FunctionName,
		UAdvancedToolsEditorBPLibrary::StaticClass()
	);
	UpdateDataTableRowFunction->AllocateDefaultPins();
	CompilerContext.MovePinLinksToIntermediate(
		*GetExecPin(),
		*UpdateDataTableRowFunction->GetExecPin()
	);
	CompilerContext.MovePinLinksToIntermediate(
		*GetThenPin(),
		*UpdateDataTableRowFunction->GetThenPin()
	);

	// DataTable pin
	UEdGraphPin * DataTablePin = UpdateDataTableRowFunction->FindPinChecked(TEXT("DataTable"));
	if (OriginalDataTablePin->LinkedTo.Num() > 0)
	{
		// Copy the connection
		CompilerContext.MovePinLinksToIntermediate(*OriginalDataTablePin, *DataTablePin);
	}
	else
	{
		// Copy literal
		DataTablePin->DefaultObject = OriginalDataTablePin->DefaultObject;
	}

	// RowName pin
	UEdGraphPin * RowNamePin = UpdateDataTableRowFunction->FindPinChecked(TEXT("RowName"));
	CompilerContext.MovePinLinksToIntermediate(*GetRowNamePin(), *RowNamePin);

	// RowData pin
	UEdGraphPin * OriginalRowDataPin = FindPinChecked(TEXT("RowData"));
	UEdGraphPin * FunctionRowDataPin = UpdateDataTableRowFunction->FindPinChecked(TEXT("RowData"));
	if (OriginalRowDataPin->LinkedTo.Num())
	{
		CompilerContext.MovePinLinksToIntermediate(*OriginalRowDataPin, *FunctionRowDataPin);
	}
	else
	{
		UK2Node_MakeStruct * MakeStructNode = CompilerContext.SpawnIntermediateNode<
			UK2Node_MakeStruct>(
			this,
			SourceGraph
		);
		MakeStructNode->StructType = GetDataTableRowStructType();
		// Suppresses "Node was probably upgraded" message
		MakeStructNode->bMadeAfterOverridePinRemoval = true;
		MakeStructNode->AllocateDefaultPins();
		for (UEdGraphPin * Pin : MakeStructNode->Pins)
		{
			if (Pin->Direction == EGPD_Output)
			{
				Pin->MakeLinkTo(FunctionRowDataPin);
				break;
			}
		}
	}

	// Set the type of the OutRow pin on this expanded node to match original
	FunctionRowDataPin->PinType                      = OriginalRowDataPin->PinType;
	FunctionRowDataPin->PinType.PinSubCategoryObject = OriginalRowDataPin->PinType.
	                                                                       PinSubCategoryObject;

	BreakAllNodeLinks();
}

FSlateIcon UK2Node_UpdateDataTableRow::GetIconAndTint(FLinearColor & OutColor) const
{
	OutColor = GetNodeTitleColor();
	static FSlateIcon Icon("EditorStyle", "Kismet.AllClasses.FunctionIcon");
	return Icon;
}

void UK2Node_UpdateDataTableRow::PostReconstructNode( )
{
	Super::PostReconstructNode();

	RefreshRowDataPinType();
}

void UK2Node_UpdateDataTableRow::ReallocatePinsDuringReconstruction(
	TArray<UEdGraphPin*> & OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);

	if (UEdGraphPin * DataTablePin = GetDataTablePin(&OldPins))
	{
		if (UDataTable * DataTable = Cast<UDataTable>(DataTablePin->DefaultObject))
		{
			// make sure to properly load the data-table object so that we can 
			// farm the "RowStruct" property from it (below, in GetDataTableRowStructType)
			PreloadObject(DataTable);
		}
	}
}

void UK2Node_UpdateDataTableRow::GetMenuActions(
	FBlueprintActionDatabaseRegistrar & ActionRegistrar) const
{
	// actions get registered under specific object-keys; the idea is that 
	// actions might have to be updated (or deleted) if their object-key is  
	// mutated (or removed)... here we use the node's class (so if the node 
	// type disappears, then the action should go with it)
	UClass * ActionKey = GetClass();
	// to keep from needlessly instantiating a UBlueprintNodeSpawner, first   
	// check to make sure that the registrar is looking for actions of this type
	// (could be regenerating actions for a specific asset, and therefore the 
	// registrar would only accept actions corresponding to that asset)
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner * NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_UpdateDataTableRow::GetMenuCategory( ) const
{
	return FText::FromString("Editor Scripting | DataTable");
	// return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Development);
}

bool UK2Node_UpdateDataTableRow::IsConnectionDisallowed(const UEdGraphPin * MyPin,
                                                        const UEdGraphPin * OtherPin,
                                                        FString &           OutReason) const
{
	if (MyPin == GetRowDataPin() && MyPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
	{
		bool bDisallowed = true;
		if (OtherPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
		{
			if (UScriptStruct * ConnectionType = Cast<UScriptStruct>(
				OtherPin->PinType.PinSubCategoryObject.Get()
			))
			{
				bDisallowed = !FDataTableEditorUtils::IsValidTableStruct(ConnectionType);
			}
		}
		else if (OtherPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
		{
			bDisallowed = false;
		}

		if (bDisallowed)
		{
			OutReason = TEXT("Must be a struct that can be used in a DataTable");
		}
		return bDisallowed;
	}
	return false;
}

void UK2Node_UpdateDataTableRow::EarlyValidation(FCompilerResultsLog & MessageLog) const
{
	Super::EarlyValidation(MessageLog);

	const UEdGraphPin * DataTablePin = GetDataTablePin();
	const UEdGraphPin * RowNamePin   = GetRowNamePin();
	if (!DataTablePin || !RowNamePin)
	{
		MessageLog.Error(*LOCTEXT("MissingPins", "Missing pins in @@").ToString(), this);
		return;
	}

	if (DataTablePin->LinkedTo.Num() == 0)
	{
		const UDataTable * DataTable = Cast<UDataTable>(DataTablePin->DefaultObject);
		if (!DataTable)
		{
			MessageLog.Error(*LOCTEXT("NoDataTable", "No DataTable in @@").ToString(), this);
			return;
		}

		if (!RowNamePin->LinkedTo.Num())
		{
			const FName CurrentName = FName(*RowNamePin->GetDefaultAsString());
			if (CurrentName == NAME_None)
			{
				MessageLog.Error(
					*LOCTEXT("NoRowName", "No RowName specified for @@").ToString(),
					this
				);
			}
		}
	}
}

void UK2Node_UpdateDataTableRow::PreloadRequiredAssets( )
{
	if (UEdGraphPin * DataTablePin = GetDataTablePin())
	{
		if (UDataTable * DataTable = Cast<UDataTable>(DataTablePin->DefaultObject))
		{
			// make sure to properly load the data-table object so that we can 
			// farm the "RowStruct" property from it (below, in GetDataTableRowStructType)
			PreloadObject(DataTable);
		}
	}
	return Super::PreloadRequiredAssets();
}

void UK2Node_UpdateDataTableRow::NotifyPinConnectionListChanged(UEdGraphPin * Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);

	if (Pin == GetRowDataPin())
	{
		UEdGraphPin * TablePin = GetDataTablePin();
		// this connection would only change the output type if the table pin is undefined
		const bool bIsTypeAuthority =
			(TablePin->LinkedTo.Num() > 0 || TablePin->DefaultObject == nullptr);
		if (bIsTypeAuthority)
		{
			RefreshRowDataPinType();
		}
	}
	else if (Pin == GetDataTablePin())
	{
		const bool bConnectionAdded = Pin->LinkedTo.Num() > 0;
		if (bConnectionAdded)
		{
			// if a connection was made, then we may need to rid ourselves of the row dropdown
			// RefreshRowNameOptions();
			// if the output connection was previously, incompatible, it now becomes the authority on this node's output type
			RefreshRowDataPinType();
		}
	}
}

UScriptStruct * UK2Node_UpdateDataTableRow::GetReturnTypeForStruct( )
{
	UScriptStruct * ReturnStructType =
		(UScriptStruct*)(GetRowDataPin()->PinType.PinSubCategoryObject.Get());

	return ReturnStructType;
}

UEdGraphPin * UK2Node_UpdateDataTableRow::GetThenPin( ) const
{
	const UEdGraphSchema_K2 * K2Schema = GetDefault<UEdGraphSchema_K2>();

	UEdGraphPin * Pin = FindPinChecked(UEdGraphSchema_K2::PN_Then);
	assumeChecked(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin * UK2Node_UpdateDataTableRow::GetDataTablePin(
	const TArray<UEdGraphPin*> * InPinsToSearch) const
{
	const TArray<UEdGraphPin*> * PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin * Pin = nullptr;
	for (UEdGraphPin * TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == GetDataTableRowHelper::DataTablePinName)
		{
			Pin = TestPin;
			break;
		}
	}
	assumeChecked(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin * UK2Node_UpdateDataTableRow::GetRowNamePin( ) const
{
	UEdGraphPin * Pin = FindPinChecked(GetDataTableRowHelper::RowNamePinName);
	assumeChecked(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin * UK2Node_UpdateDataTableRow::GetRowDataPin(
	const TArray<UEdGraphPin*> * InPinsToSearch) const
{
	const TArray<UEdGraphPin*> * PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin * Pin = nullptr;
	for (UEdGraphPin * TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == GetDataTableRowHelper::RowDataPinName)
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UScriptStruct * UK2Node_UpdateDataTableRow::GetDataTableRowStructType( ) const
{
	UScriptStruct * RowStructType = nullptr;

	UEdGraphPin * DataTablePin = GetDataTablePin();
	if (DataTablePin && DataTablePin->DefaultObject != nullptr && DataTablePin->LinkedTo.Num() == 0)
	{
		if (const UDataTable * DataTable = Cast<const UDataTable>(DataTablePin->DefaultObject))
		{
			RowStructType = DataTable->RowStruct;
		}
	}

	if (RowStructType == nullptr)
	{
		UEdGraphPin * ResultPin = GetRowDataPin();
		if (ResultPin && ResultPin->LinkedTo.Num() > 0)
		{
			RowStructType = Cast<UScriptStruct>(
				ResultPin->LinkedTo[0]->PinType.PinSubCategoryObject.Get()
			);
			for (int32 LinkIndex = 1; LinkIndex < ResultPin->LinkedTo.Num(); ++LinkIndex)
			{
				UEdGraphPin *   Link     = ResultPin->LinkedTo[LinkIndex];
				UScriptStruct * LinkType = Cast<UScriptStruct>(
					Link->PinType.PinSubCategoryObject.Get()
				);

				if (RowStructType->IsChildOf(LinkType))
				{
					RowStructType = LinkType;
				}
			}
		}
	}
	return RowStructType;
}

void UK2Node_UpdateDataTableRow::SetPinToolTip(UEdGraphPin & MutatablePin,
                                               const FText & PinDescription) const
{
	MutatablePin.PinToolTip = UEdGraphSchema_K2::TypeToText(MutatablePin.PinType).ToString();

	UEdGraphSchema_K2 const * const K2Schema = Cast<const UEdGraphSchema_K2>(GetSchema());
	if (K2Schema != nullptr)
	{
		MutatablePin.PinToolTip += TEXT(" ");
		MutatablePin.PinToolTip += K2Schema->GetPinDisplayName(&MutatablePin).ToString();
	}

	MutatablePin.PinToolTip += FString(TEXT("\n")) + PinDescription.ToString();
}

void UK2Node_UpdateDataTableRow::SetInputTypeForStruct(UScriptStruct * NewRowStruct)
{
	UScriptStruct * OldRowStruct = GetReturnTypeForStruct();
	if (NewRowStruct != OldRowStruct)
	{
		UEdGraphPin * ResultPin = GetRowDataPin();

		if (ResultPin->SubPins.Num() > 0)
		{
			GetSchema()->RecombinePin(ResultPin);
		}

		// NOTE: purposefully not disconnecting the ResultPin (even though it changed type)... we want the user to see the old
		//       connections, and incompatible connections will produce an error (plus, some super-struct connections may still be valid)
		ResultPin->PinType.PinSubCategoryObject = NewRowStruct;
		ResultPin->PinType.PinCategory          = (NewRowStruct == nullptr)
			                                          ? UEdGraphSchema_K2::PC_Wildcard
			                                          : UEdGraphSchema_K2::PC_Struct;

		CachedNodeTitle.Clear();
	}
}

void UK2Node_UpdateDataTableRow::RefreshRowDataPinType( )
{
	UScriptStruct * OutputType = GetDataTableRowStructType();
	SetInputTypeForStruct(OutputType);
}

// void UK2Node_UpdateDataTableRow::RefreshRowNameOptions()
// {
// 	// When the DataTable pin gets a new value assigned, we need to update the Slate UI so that SGraphNodeCallParameterCollectionFunction will update the ParameterName drop down
// 	UEdGraph* Graph = GetGraph();
// 	Graph->NotifyGraphChanged();
// }

#undef LOCTEXT_NAMESPACE
