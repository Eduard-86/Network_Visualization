#include "GraphEnchantments.h"

#include "AssumeChecked.h"
#include "BlueprintEditor.h"
#include "BlueprintEditorModule.h"
#include "EdGraphSchema_K2.h"
#include "GraphCommands.h"
#include "GraphEditorModule.h"
#include "GraphNodeToken.h"
#include "K2Node_CallFunction.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_VariableGet.h"
#include "KismetCompiler.h"
#include "SGraphPanel.h"
#include "ScopedTransaction.h"
#include "Delegates/IDelegateInstance.h"
#include "EdGraph/EdGraph.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Logging/MessageLog.h"
#include "Modules/ModuleManager.h"
#include "Node/Enchanted/EnchantedNode.h"
#include "Node/Enchanted/K2Node_CallFunctionEnchanted.h"
#include "Node/Enchanted/K2Node_CommutativeAssociativeBinaryOperatorEnchanted.h"
#include "Node/Enchanted/K2Node_MacroInstanceEnchanted.h"
#include "Node/Enchanted/K2Node_VariableGetEnchanted.h"
#include "Widgets/Notifications/SNotificationList.h"

FDelegateHandle                FGraphEnchantments::ExtendMenusHandle;
FDelegateHandle                FGraphEnchantments::OnReplacingNodeCreatedHandle;
TArray<const IEnchantedNode *> FGraphEnchantments::Enchantments;
TSharedPtr<FUICommandList>     FGraphEnchantments::GraphCommands;
FDelegateHandle                FGraphEnchantments::OnBlueprintEditorOpenedHandle;
TSharedPtr<FGraphEnchantments> FGraphEnchantments::InputProcessor;
UK2Node *                      FGraphEnchantments::NodeToReplace;
TSharedPtr<SNotificationItem>  FGraphEnchantments::BranchEmbeddingFailureNotification;

#define LOCTEXT_NAMESPACE "BlueprintEnchantments"

// Lifecycle //
/*************************************************************************************************/

#pragma region Lifecycle

void FGraphEnchantments::StartUp( )
{
	if (IsRunningCommandlet( ) || !FSlateApplication::IsInitialized( ))
		return;

	// Setup UI commands
	{
		InputProcessor = MakeShared<FGraphEnchantments>( );
		FSlateApplication::Get( ).RegisterInputPreProcessor(InputProcessor);
		FEnchantedGraphCommands::Register( );

		const FEnchantedGraphCommands & Commands = FEnchantedGraphCommands::Get( );
		GraphCommands                            = MakeShared<FUICommandList>( );
		GraphCommands->MapAction(
			Commands.SplitNextPinIntoExecs,
			FExecuteAction::CreateStatic(SplitNextPinIntoExecs),
			EUIActionRepeatMode::RepeatDisabled
		);
		GraphCommands->MapAction(
			Commands.ReplaceNode,
			FExecuteAction::CreateStatic(ReplaceNode),
			EUIActionRepeatMode::RepeatDisabled
		);
	}

	// Setup menu extender
	{
		// @formatter:wrap_lines false
		FGraphEditorModule & GraphEditor = FModuleManager::GetModuleChecked<FGraphEditorModule>("GraphEditor");
		TArray<FGraphEditorModule::FGraphEditorMenuExtender_SelectedNode> & Extenders = GraphEditor.GetAllGraphEditorContextMenuExtender( );
		const auto Extender = FGraphEditorModule::FGraphEditorMenuExtender_SelectedNode::CreateStatic(&FGraphEnchantments::GetSelectedNodeMenusExtender);
		ExtendMenusHandle = Extender.GetHandle( );
		// @formatter:wrap_lines restore

		Extenders.Add(Extender);
	}

	// Listen for new Library editors
	{
		FBlueprintEditorModule & Kismet = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");
		OnBlueprintEditorOpenedHandle   = Kismet.OnBlueprintEditorOpened( ).AddStatic(OnBlueprintEditorOpened);
	}

	// Register enchantments
	{
		// Warning: order matters
		Enchantments.Push(GetDefault<UK2Node_CommutativeAssociativeBinaryOperatorEnchanted>( ));
		Enchantments.Push(GetDefault<UK2Node_VariableGetEnchanted>( ));
		Enchantments.Push(GetDefault<UK2Node_CallFunctionEnchanted>( ));
		Enchantments.Push(GetDefault<UK2Node_MacroInstanceEnchanted>( ));
	}
}

void FGraphEnchantments::ShutDown( )
{
	if (IsRunningCommandlet( ) || !FSlateApplication::IsInitialized( ))
		return;

	// Unregister input processor
	{
		FSlateApplication::Get( ).UnregisterInputPreProcessor(InputProcessor);
	}

	// Unregister menu extender
	if (ExtendMenusHandle.IsValid( ))
	{
		// @formatter:wrap_lines false
		FGraphEditorModule & GraphEditor = FModuleManager::GetModuleChecked<FGraphEditorModule>("GraphEditor");
		TArray<FGraphEditorModule::FGraphEditorMenuExtender_SelectedNode> & Extenders = GraphEditor.GetAllGraphEditorContextMenuExtender( );
		Extenders.RemoveAll(
			[](const FGraphEditorModule::FGraphEditorMenuExtender_SelectedNode & Delegate)-> bool
			{
				return Delegate.GetHandle( ) == ExtendMenusHandle;
			}
		);
		// @formatter:wrap_lines restore

		FEnchantedGraphCommands::Unregister( );
	}

	// Stop listening for new BP editors
	if (OnBlueprintEditorOpenedHandle.IsValid( ))
	{
		FBlueprintEditorModule & Kismet = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");
		Kismet.OnBlueprintEditorOpened( ).Remove(OnBlueprintEditorOpenedHandle);
	}
}

bool FGraphEnchantments::HandleKeyDownEvent(FSlateApplication & SlateApp, const FKeyEvent & InKeyEvent)
{
	GraphCommands->ProcessCommandBindings(InKeyEvent);
	return false;
}

#pragma endregion ~ Lifecycle

// UI Extension //
/*************************************************************************************************/

#pragma region UI Extension

TSharedRef<FExtender> FGraphEnchantments::GetSelectedNodeMenusExtender(
	const TSharedRef<FUICommandList> CommandList,
	const UEdGraph *                 Graph,
	const UEdGraphNode *             Node,
	const UEdGraphPin *              Pin,
	const bool                       bIsEditable
)
{
	// Also available:
	// "EdGraphSchemaOrganization"

	TSharedRef<FExtender> Extender = MakeShared<FExtender>( );

	if (false
		|| !Node
		|| !Graph
		|| !Pin
		|| Node->GetGraph( ) != Graph
		|| Pin->GetOwningNode( ) != Node
	)
	{
		return Extender;
	}

	const UK2Node * K2Node = Cast<UK2Node>(Node);
	if (!K2Node)
		return Extender;

	if (Graph && Node && Pin && Pin->Direction == EGPD_Output && IsNodeEnchantable(*K2Node))
	{
		switch (GetAvailablePinEnchantments(K2Node, Pin))
		{
			case Expansion:
			{
				CommandList->MapAction(
					FEnchantedGraphCommands::Get( ).SplitPinIntoExecs,
					FExecuteAction::CreateStatic(
						&FGraphEnchantments::SplitPinIntoExecs,
						// They should not be edited now, but not later
						const_cast<UEdGraph *>(Graph),
						const_cast<UK2Node *>(K2Node),
						const_cast<UEdGraphPin *>(Pin),
						bIsEditable
					)
				);

				Extender->AddMenuExtension(
					"EdGraphSchemaPinActions",
					EExtensionHook::After,
					CommandList,
					FMenuExtensionDelegate::CreateStatic(&FGraphEnchantments::AddPinExpansionEntry)
				);
				break;
			}
			case Recombination:
			{
				CommandList->MapAction(
					FEnchantedGraphCommands::Get( ).RecombinePinFromExecs,
					FExecuteAction::CreateStatic(
						&FGraphEnchantments::RecombinePinFromExecs,
						const_cast<UEdGraph *>(Graph),
						const_cast<UK2Node *>(K2Node),
						const_cast<UEdGraphPin *>(Pin),
						bIsEditable
					)
				);

				Extender->AddMenuExtension(
					"EdGraphSchemaPinActions",
					EExtensionHook::After,
					CommandList,
					FMenuExtensionDelegate::CreateStatic(&FGraphEnchantments::AddPinRecombinationEntry)
				);
				break;
			}
			case None:
			default: ;
		}
	}

	return Extender;
}

void FGraphEnchantments::AddPinExpansionEntry(
	FMenuBuilder & MenuBuilder
)
{
	MenuBuilder.AddMenuEntry(FEnchantedGraphCommands::Get( ).SplitPinIntoExecs);
}

void FGraphEnchantments::AddPinRecombinationEntry(
	FMenuBuilder & MenuBuilder
)
{
	MenuBuilder.AddMenuEntry(FEnchantedGraphCommands::Get( ).RecombinePinFromExecs);
}

#pragma endregion ~ UI Extension

// Enchantment //
/*************************************************************************************************/

#pragma region Enchantment

void FGraphEnchantments::SplitNextPinIntoExecs( )
{
	UK2Node * Node = GetSelectedNode( );
	if (!Node)
		return;

	FScopedTransaction Transaction(GetSplittingTransactionDescription( ));

	if (const auto Enchantmnet = Cast<IEnchantedNode>(Node))
	{
		if (const auto NewNode = Enchantmnet->SplitNextPin( ))
		{
			ReplaceNode(*Node->GetGraph( ), *Node, *NewNode);
			return;
		}
	}
	else
	{
		for (const IEnchantedNode * Candidate : Enchantments)
		{
			if (const auto NewNode = Candidate->Enchant(*Node))
			{
				ReplaceNode(*Node->GetGraph( ), *Node, *NewNode);
				return;
			}
		}
	}

	// Probably excess
	Transaction.Cancel( );
	{
		FNotificationInfo NotificationInfo(LOCTEXT("Can't embed condition notification", "Can't embed condition!"));
		NotificationInfo.Image         = FCoreStyle::Get( ).GetBrush(TEXT("MessageLog.Error"));
		NotificationInfo.bUseLargeFont = false;

		FSlateNotificationManager::Get( ).AddNotification(NotificationInfo);
	}
}

/*
 * I just want a local loop, I don't want any kind of global graph management or whatever
 * Probably should've cracked UEdGraph to read multicast delegate contents instead -_-
 */

union FExposedDelegateHandle
{
	FExposedDelegateHandle( )
		: AsHandle( )
	  , AsRawId(0) {}

	FDelegateHandle AsHandle;
	uint64          AsRawId;
};


static_assert(sizeof(FDelegateHandle) == sizeof(uint64), "FDelegateHandle<->uint64 structural mismatch");
static_assert(sizeof(FExposedDelegateHandle) == sizeof(uint64), "FDelegateHandle<->uint64 structural mismatch");


struct FLambda
{
	FExposedDelegateHandle Handle;

	void operator()(const FEdGraphEditAction & Action) const
	{
		assumeChecked(Handle.AsHandle.IsValid());
		FGraphEnchantments::OnReplacingNodeCreated(Action, Handle.AsHandle);
	}
};


void FGraphEnchantments::ReplaceNode( )
{
	SGraphPanel * Graph = GetFocusedGraph( );
	if (!Graph)
		return;

	UK2Node * Node = GetSelectedNode(*Graph);
	if (!Node)
		return;

	FLambda                Lambda;
	FExposedDelegateHandle ExposedHandle;

	// Force UE generate a new handle to read last valid ID
	auto Delegate          = FOnGraphChanged::FDelegate::CreateLambda([](const FEdGraphEditAction &) {});
	ExposedHandle.AsHandle = Delegate.GetHandle( );

	// Store ID of next generated delegate into the lambda
	Lambda.Handle.AsRawId = ExposedHandle.AsRawId == UINT64_MAX ? 1 : ExposedHandle.AsRawId + 1;

	// Create the real callback, and pass lambda inside it
	Delegate.BindLambda(Lambda);
	check(Delegate.GetHandle() == Lambda.Handle.AsHandle);

	// Send the callback forth
	NodeToReplace = Node;
	Node->GetGraph( )->AddOnGraphChangedHandler(Delegate);

	FSlateApplication & Slate       = FSlateApplication::Get( );
	const auto          ContextMenu = Graph->SummonContextMenu(
		Slate.GetCursorPos( ),
		FVector2D(Node->NodePosX, Node->NodePosY),
		nullptr,
		nullptr,
		TArray<UEdGraphPin *>( )
	);
	Slate.SetAllUserFocus(ContextMenu);
}

void FGraphEnchantments::OnReplacingNodeCreated(
	const FEdGraphEditAction & Action,
	const FDelegateHandle      ExecutorHandle
)
{
	check(ExecutorHandle.IsValid());
	Action.Graph->RemoveOnGraphChangedHandler(ExecutorHandle);

	if (!NodeToReplace || Action.Action != GRAPHACTION_AddNode) return;

	assumeChecked(Action.Nodes.Num() == 1);
	UEdGraphNode * NewNode = const_cast<UEdGraphNode *>(Action.Nodes[Action.Nodes.begin( ).GetId( )]);
	UEdGraphNode * OldNode = NodeToReplace;
	NodeToReplace          = nullptr;

	assumeChecked(GEditor);
	const auto TransactionIndex = GEditor->BeginTransaction(
		TEXT("Blueprint Enchantments"),
		LOCTEXT("Replace Node Transaction", "Replace Node"),
		Action.Graph
	);
	check(TransactionIndex >= 0);

	FTimerHandle TimerHandle;
	auto         Lambda = [OldNode, NewNode]( )
	{
		SynchronizeSplittedStructPins(*OldNode, *NewNode);
		ReplaceNode(
			*OldNode->GetGraph( ),
			*OldNode,
			*NewNode
		);

		assumeChecked(GEditor);
		GEditor->EndTransaction( );
	};
	GEditor->GetTimerManager( )->SetTimer(TimerHandle, FTimerDelegate::CreateLambda(Lambda), SMALL_NUMBER, false);
}

void FGraphEnchantments::SplitPinIntoExecs(
	UEdGraph *    Graph,
	UK2Node *     Node,
	UEdGraphPin * Pin,
	bool          bIsEditable
)
{
	if (UNLIKELY(!Graph || !Node || !Pin))
		return;

	FScopedTransaction Transaction((GetSplittingTransactionDescription( )));

	for (const IEnchantedNode * Candidate : Enchantments)
	{
		if (auto NewNode = Candidate->Enchant(*Node, *Pin))
		{
			if (NewNode.IsSet( ))
			{
				if (NewNode.GetValue( ) != nullptr)
				{
					ReplaceNode(*Graph, *Node, *NewNode.GetValue( ));
				}
				return;
			}
		}
	}

	// Probably excess
	Transaction.Cancel( );
}

void FGraphEnchantments::RecombinePinFromExecs(
	UEdGraph *    Graph,
	UK2Node *     Node,
	UEdGraphPin * Pin,
	bool          bIsEditable
)
{
	if (UNLIKELY(!Graph || !Node || !Pin))
		return;

	if (const auto Enchantment = Cast<IEnchantedNode>(Node))
	{
		FScopedTransaction Transaction(LOCTEXT("Recombine Pin from Execs transaction", "Recombine Pin from Execs"));
		ReplaceNode(*Graph, *Node, Enchantment->Disenchant( ));
	}
}

bool FGraphEnchantments::IsPinSplittable(
	const UEdGraphPin & Pin
)
{
	const auto & Category = Pin.PinType.PinCategory;

	// TODO Enums and splitted pins are not supported atm
	return
			Pin.Direction == EGPD_Output
			&& Pin.ParentPin == nullptr
			&& Category == UEdGraphSchema_K2::PC_Boolean;
}

EPinEnchantment FGraphEnchantments::GetAvailablePinEnchantments(
	const UK2Node *     Node,
	const UEdGraphPin * Pin
)
{
	if (!Pin || !Node)
		return None;

	if (Pin->Direction == EGPD_Input)
		return None;

	if (IsPinSplittable(*Pin))
		return Expansion;

	const bool bIsExecPin       = Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec;
	const bool bIsOutPin        = Pin->Direction == EGPD_Output;
	const bool bIsThenPin       = bIsExecPin && bIsOutPin;
	const bool bIsNodeEnchanted = Node->IsA<UK2Node_CallFunctionEnchanted>( )
			|| Node->IsA<UK2Node_VariableGetEnchanted>( )
			|| Node->IsA<UK2Node_CommutativeAssociativeBinaryOperatorEnchanted>( )
			|| Node->IsA<UK2Node_MacroInstanceEnchanted>( );
	if (bIsThenPin && bIsNodeEnchanted)
		return Recombination;

	return None;
}

bool FGraphEnchantments::IsNodeEnchantable(
	const UK2Node & Node
)
{
	bool Result = false;

	if (Cast<IEnchantedNode>(&Node))
	{
		Result = true;
	}
	else
	{
		for (const IEnchantedNode * Candidate : Enchantments)
		{
			if (Candidate->CanEnchant(Node))
			{
				Result = true;
				break;
			}
		}
	}

	return Result;
}

UEdGraphPin * FGraphEnchantments::GetNextExpandablePin(
	const UK2Node & Node,
	const FName     NameOfPinAfterExpandedOne
)
{
	const int32 NumPins = Node.Pins.Num( );

	for (int i = 0; i < NumPins; ++i)
	{
		const UEdGraphPin * Pin = Node.Pins[i];
		assumeChecked(Pin);

		if (Pin->PinName == NameOfPinAfterExpandedOne)
		{
			for (i; i < NumPins; ++i)
			{
				PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS
				UEdGraphPin * Pin = Node.Pins[i];
				assumeChecked(Pin);

				if (IsPinSplittable(*Pin))
				{
					return Pin;
				}
				PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
			}
		}
	}

	return nullptr;
}

#pragma endregion ~ Enchantment

// Utility //
/*************************************************************************************************/

#pragma region Utility

void FGraphEnchantments::ReplaceNode(
	UEdGraph &     Graph,
	UEdGraphNode & OldNode,
	UEdGraphNode & NewNode
)
{
	NewNode.NodePosX = OldNode.NodePosX;
	NewNode.NodePosY = OldNode.NodePosY;

	for (UEdGraphPin * NewPin : NewNode.Pins)
	{
		if (UEdGraphPin * OldPin = OldNode.FindPin(NewPin->PinName, NewPin->Direction))
		{
			auto CopyWildcardType = [&OldPin, &NewPin]( )-> bool
			{
				if (true
					&& NewPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard
					&& OldPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard
				)
				{
					NewPin->PinType = OldPin->PinType;
					return true;
				}

				return false;
			};

			if (false
				|| OldPin->PinType == NewPin->PinType
				|| CopyWildcardType( ))
			{
				if (OldPin->LinkedTo.Num( ))
				{
					for (UEdGraphPin * Link : OldPin->LinkedTo)
					{
						NewPin->MakeLinkTo(Link);
					}
					OldPin->BreakAllPinLinks( );
				}
				else
				{
					NewPin->DefaultObject    = OldPin->DefaultObject;
					NewPin->DefaultTextValue = OldPin->DefaultTextValue;
					NewPin->DefaultValue     = OldPin->DefaultValue;
				}
			}
		}
	}

	Graph.RemoveNode(&OldNode);
}

void FGraphEnchantments::SynchronizeSplittedStructPins(
	const UEdGraphNode & From,
	UEdGraphNode &       To
)
{
	assumeChecked(From.GetSchema() == To.GetSchema());
	assumeChecked(From.GetGraph( ) && To.GetGraph());

	const UEdGraphSchema * Schema  = To.GetSchema( );
	const auto             Iterate = [&]( )-> bool
	{
		for (const UEdGraphPin * SamplePin : From.Pins)
		{
			assumeChecked(SamplePin)
			if (SamplePin->SubPins.Num( ))
			{
				if (UEdGraphPin * TargetPin = To.FindPin(SamplePin->PinName, SamplePin->Direction))
				{
					if (TargetPin->SubPins.Num( ) == 0 && TargetPin->PinType == SamplePin->PinType)
					{
						Schema->SplitPin(TargetPin);
						return true;
					}
				}
			}
		}

		return false;
	};

	while (Iterate( ));
}

void FGraphEnchantments::CopyWildcardTypes(const UEdGraphNode & From, UEdGraphNode & To)
{
	for (UEdGraphPin * ToPin : To.Pins)
	{
		if (ToPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
		{
			if (const UEdGraphPin * FromPin = From.FindPin(ToPin->PinName, ToPin->Direction))
			{
				if (FromPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
				{
					ToPin->PinType = FromPin->PinType;
				}
			}
		}
	}
}

FBlueprintEditor * FGraphEnchantments::GetActiveBlueprintEditor( )
{
	const auto Kismet = static_cast<FBlueprintEditorModule *>(FModuleManager::Get( ).GetModule("Kismet"));
	if (!Kismet)
		return nullptr;


	const FTabManager & ActiveTabManager =
#if ENGINE_MAJOR_VERSION >= 5
			*FGlobalTabmanager::Get( )->GetActiveTab( )->GetTabManagerPtr( ).Get( );
	assumeChecked(&ActiveTabManager);
#else
		FGlobalTabmanager::Get( )->GetActiveTab( )->GetTabManager( ).Get( );
#endif
	const TArray<TSharedRef<IBlueprintEditor>> Editors = Kismet->GetBlueprintEditors( );
	for (const auto & Editor : Editors)
	{
		if (Editor->GetTabManager( ).Get( ) == &ActiveTabManager && Editor->GetToolkitFName( ) == "BlueprintEditor")
		{
			return static_cast<FBlueprintEditor *>(&Editor.Get( ));
		}
	}

	return nullptr;
}

SGraphPanel * FGraphEnchantments::GetFocusedGraph( )
{
	SWidget * FocusedWidget = FSlateApplication::Get( ).GetKeyboardFocusedWidget( ).Get( );
	if (!FocusedWidget)
		return nullptr;

	const FName FocusedWidgetType = FocusedWidget->GetType( );
	if (FocusedWidgetType != ((SGraphPanel *)nullptr, "SGraphPanel"))
		return nullptr;

	return static_cast<SGraphPanel *>(FocusedWidget);
}

UK2Node * FGraphEnchantments::GetSelectedNode(const SNodePanel & GraphWidget)
{
	const auto & SelectedNodes = GraphWidget.SelectionManager.SelectedNodes;
	if (SelectedNodes.Num( ) != 1)
		return nullptr;

	UObject * SelectedNodeObject = SelectedNodes[SelectedNodes.begin( ).GetId( )];
	UK2Node * SelectedNode       = Cast<UK2Node>(SelectedNodeObject);
	if (!SelectedNode)
		return nullptr;

	const UEdGraphSchema * Schema = SelectedNode->GetSchema( );
	const UEdGraph *       Graph  = SelectedNode->GetGraph( );
	if (!Graph || !Schema || !Cast<UEdGraphSchema_K2>(Schema))
		return nullptr;

	return SelectedNode;
}

UK2Node * FGraphEnchantments::GetSelectedNode( )
{
	const SGraphPanel * FocusedGraph = GetFocusedGraph( );

	return FocusedGraph ? GetSelectedNode(*FocusedGraph) : nullptr;
}

FName FGraphEnchantments::GetNextPinName(
	const UK2Node & Node,
	const FName     PinName
)
{
	auto & Pins = Node.Pins;
	for (int i = 0; i < Pins.Num( ); i++)
	{
		if (Pins[i]->PinName == PinName && Pins.IsValidIndex(i + 1))
		{
			return Pins[i + 1]->PinName;
		}
	}

	return NAME_None;
}

#pragma endregion ~ Utility

// Node expansion //
/*************************************************************************************************/

#pragma region Node expansion

void FGraphEnchantments::NotifyBranchEmbeddingFailed(UK2Node & Node)
{
	FMessageLog("EditorErrors")
			.Warning(LOCTEXT("Pin Splitting Error", "Couldn't split pin into Execs: "))
			->AddToken(FGraphNodeToken::Create(Node));

	if (BranchEmbeddingFailureNotification.IsValid( ))
		return;

	FNotificationInfo NotificationInfo(LOCTEXT("Pin Splitting Error Notification", "Branch embedding error!"));
	NotificationInfo.Image          = FCoreStyle::Get( ).GetBrush(TEXT("MessageLog.Warning"));
	NotificationInfo.bFireAndForget = false;
	NotificationInfo.bUseLargeFont  = false;
	NotificationInfo.Hyperlink      = FSimpleDelegate::CreateLambda(
		[]( )
		{
			FMessageLog("EditorErrors").Open( );
			BranchEmbeddingFailureNotification->Fadeout( );
			BranchEmbeddingFailureNotification.Reset( );
		}
	);
	NotificationInfo.ButtonDetails.Emplace(
		LOCTEXT("Pin Splitting Error Notification Dismiss Button", "Dismiss"),
		FText::GetEmpty( ),
		FSimpleDelegate::CreateLambda(
			[]( )
			{
				BranchEmbeddingFailureNotification->Fadeout( );
				BranchEmbeddingFailureNotification.Reset( );
			}
		),
		SNotificationItem::CS_None
	);
	BranchEmbeddingFailureNotification = FSlateNotificationManager::Get( ).AddNotification(NotificationInfo);
}

UEdGraphPin * FGraphEnchantments::GetOriginalPin(
	const UK2Node & Node,
	const FName     PinName
)
{
	if (UEdGraphPin * OriginalPin = Node.FindPin(PinName, EGPD_Output))
	{
		if (OriginalPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
		{
			return OriginalPin;
		}
	}

	return nullptr;
}

bool FGraphEnchantments::IsOriginalPinPresent(
	const UK2Node & Node,
	const FName     PinName
)
{
	return GetOriginalPin(Node, PinName) != nullptr;
}

void FGraphEnchantments::ExpandPin(
	UK2Node &     Node,
	UEdGraphPin & ExpandedPin
)
{
	UK2Node::FCreatePinParams Params;
	Params.Index = 0;

	const auto TruePin = Node.CreatePin(
		EGPD_Output,
		UEdGraphSchema_K2::PC_Exec,
		MakeFalseBranchName(ExpandedPin.PinName),
		Params
	);
	const auto FalsePin = Node.CreatePin(
		EGPD_Output,
		UEdGraphSchema_K2::PC_Exec,
		MakeTrueBranchName(ExpandedPin.PinName),
		Params
	);

	TruePin->PinToolTip = FalsePin->PinToolTip = ExpandedPin.PinToolTip;

	Node.RemovePin(&ExpandedPin);
}

void FGraphEnchantments::FinalizeNodeExpansion(
	FKismetCompilerContext & CompilerContext,
	UK2Node &                ExpandedNode,
	UK2Node &                ReplacingNode,
	UEdGraphPin &            BranchCondition
)
{
	const auto & BranchNode = BeginBranchConstruction(CompilerContext, ExpandedNode, BranchCondition);

	UEdGraphPin & MyExecutionPin     = *ExpandedNode.FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
	auto &        BranchExecutionPin = *BranchNode.FindPinChecked(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
	CompilerContext.MovePinLinksToIntermediate(MyExecutionPin, BranchExecutionPin);

	TransferPinLinks(CompilerContext, ExpandedNode, ReplacingNode);
}

UK2Node_IfThenElse & FGraphEnchantments::BeginBranchConstruction(
	FKismetCompilerContext & CompilerContext,
	UK2Node &                ExpandedNode,
	UEdGraphPin &            ConditionPin
)
{
	// Create branch node
	auto & BranchNode = *CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(&ExpandedNode);
	BranchNode.AllocateDefaultPins( );

	// Get all manipulated branch pins
	UEdGraphPin & TrueBranchPin      = *BranchNode.FindPinChecked(UEdGraphSchema_K2::PN_Then, EGPD_Output);
	UEdGraphPin & FalseBranchPin     = *BranchNode.FindPinChecked(UEdGraphSchema_K2::PN_Else, EGPD_Output);
	UEdGraphPin & BranchConditionPin = *BranchNode.FindPinChecked(UEdGraphSchema_K2::PN_Condition, EGPD_Input);

	// Get my manipulated pins
	UEdGraphPin & MyTrueBranchPin = *ExpandedNode.FindPinChecked(MakeTrueBranchName(ConditionPin.PinName), EGPD_Output);
	UEdGraphPin & MyFalseBranchPin = *ExpandedNode.FindPinChecked(
		MakeFalseBranchName(ConditionPin.PinName),
		EGPD_Output
	);

	CompilerContext.MovePinLinksToIntermediate(MyTrueBranchPin, TrueBranchPin);
	CompilerContext.MovePinLinksToIntermediate(MyFalseBranchPin, FalseBranchPin);
	ConditionPin.MakeLinkTo(&BranchConditionPin);

	return BranchNode;
}

void FGraphEnchantments::TransferPinLinks(
	FKismetCompilerContext & CompilerContext,
	UK2Node &                ExpandedNode,
	UK2Node &                ReplacingNode
)
{
	// If dealing with a pure function, then could remove all exec pins at this point to reduce cycle misses
	// BUT this will force compiler to warn about an impure function with no exec pins, so don't
	for (UEdGraphPin * Pin : ReplacingNode.Pins)
	{
		if (UEdGraphPin * MyPin = ExpandedNode.FindPin(Pin->PinName, Pin->Direction))
		{
			if (Pin->PinType == MyPin->PinType)
			{
				CompilerContext.MovePinLinksToIntermediate(*MyPin, *Pin);
			}
		}
	}

	// Just in case
	ExpandedNode.BreakAllNodeLinks( );
}

#pragma endregion ~ Node expansion

// Branch name handling //
/*************************************************************************************************/

#pragma region Branch name handling

bool FGraphEnchantments::IsGenericPinName(
	const FName PinName
)
{
	return
			PinName.ToString( ).Len( ) <= 1
			|| PinName == "ReturnValue"
			|| PinName == "Result"
			|| PinName == "Value"
			|| PinName == "Res"
			|| PinName == "Ret"
			|| PinName == "Val";
}

FName FGraphEnchantments::MakeTrueBranchName( )
{
	static FName DefaultTrueBranchName("True");
	return DefaultTrueBranchName;
}

FName FGraphEnchantments::MakeFalseBranchName( )
{
	static FName DefaultFalseBranchName("False");
	return DefaultFalseBranchName;
}

FName FGraphEnchantments::MakeTrueBranchName(
	const FName PinName
)
{
	if (IsGenericPinName(PinName))
	{
		return MakeTrueBranchName( );
	}

	return PinName;
}

FName FGraphEnchantments::MakeFalseBranchName(
	const FName PinName
)
{
	if (IsGenericPinName(PinName))
	{
		return MakeFalseBranchName( );
	}

	return FName(FString(TEXT("Not ")) + PinName.ToString( ));
}

#pragma endregion ~ Branch name handling

//  //
/*************************************************************************************************/

void FGraphEnchantments::OnBlueprintEditorOpened(
	const EBlueprintType BlueprintType
)
{
	if (BlueprintType != BPTYPE_FunctionLibrary)
		return;

	const FBlueprintEditorModule & Kismet = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");
	for (const auto & Editor : Kismet.GetBlueprintEditors( ))
	{
		if (Editor->GetToolkitFName( ) != "BlueprintEditor")
			continue;

		FBlueprintEditor & BPEditor = static_cast<FBlueprintEditor &>(Editor.Get( ));
		if (!BPEditor.IsEditingSingleBlueprint( ))
			continue;

		UBlueprint * BlueprintObj = BPEditor.GetBlueprintObj( );
		if (BlueprintObj && BlueprintObj->BlueprintType == BPTYPE_FunctionLibrary)
		{
			BlueprintObj->SetObjectBeingDebugged(BlueprintObj->GeneratedClass->ClassDefaultObject);
		}
	}
}

FText FGraphEnchantments::GetSplittingTransactionDescription( )
{
	return LOCTEXT("Split Pin into Execs Transaction", "Split Pin into Execs");
}

#undef LOCTEXT_NAMESPACE
