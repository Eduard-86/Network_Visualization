#pragma once

#include "Framework/Application/IInputProcessor.h"
#include "Templates/SharedPointer.h"

class SNodePanel;
class SGraphPanel;
class IEnchantedNode;
class FBlueprintEditor;
class FKismetCompilerContext;

class UK2Node_CallFunctionEnchanted;
class UK2Node_CallFunction;
class UK2Node_IfThenElse;
class IK2Node_AddPinInterface;
class UK2Node_CommutativeAssociativeBinaryOperatorEnchanted;
class UK2Node_CommutativeAssociativeBinaryOperator;

class UEdGraph;
class UK2Node;
class UEdGraphPin;

class FExtender;
class FMenuBuilder;
class FUICommandList;
class FUICommandInfo;

class FDelegateHandle;

//  //
/*************************************************************************************************/

enum EPinEnchantment
{
	None,
	Expansion,
	Recombination,
};


//  //
/*************************************************************************************************/

class FGraphEnchantments final : public IInputProcessor
{
public:
#pragma region Lifecycle

	static void StartUp( );
	static void ShutDown( );

#pragma endregion ~ Lifecycle

#pragma region Input

	virtual void Tick(const float DeltaTime, FSlateApplication & SlateApp, TSharedRef<ICursor> Cursor) override {}
	virtual bool HandleKeyDownEvent(FSlateApplication & SlateApp, const FKeyEvent & InKeyEvent) override;
	virtual const TCHAR * GetDebugName( ) const override { return TEXT("Enchanted Input Processor"); }

	static void SplitNextPinIntoExecs( );

	static void ReplaceNode( );
	static void OnReplacingNodeCreated(const FEdGraphEditAction & Action, const FDelegateHandle ExecutorHandle);

#pragma endregion ~ Input

#pragma region UI Extension

	/** */
	static TSharedRef<FExtender> GetSelectedNodeMenusExtender(
		const TSharedRef<FUICommandList> CommandList,
		const UEdGraph *                 Graph,
		const UEdGraphNode *             Node,
		const UEdGraphPin *              Pin,
		bool                             bIsEditable
	);

	/** */
	static void AddPinExpansionEntry(FMenuBuilder & MenuBuilder);

	/** */
	static void AddPinRecombinationEntry(FMenuBuilder & MenuBuilder);

#pragma endregion ~ UI Extension

#pragma region Enchantment

	/** Context menu handler */
	static void SplitPinIntoExecs(UEdGraph * Graph, UK2Node * Node, UEdGraphPin * Pin, bool bIsEditable);

	/** Context menu handler */
	static void RecombinePinFromExecs(UEdGraph * Graph, UK2Node * Node, UEdGraphPin * Pin, bool bIsEditable);

	// Utility

	/** */
	static bool IsPinSplittable(const UEdGraphPin & Pin);

	/** */
	static EPinEnchantment GetAvailablePinEnchantments(
		const UK2Node *     Node,
		const UEdGraphPin * Pin
	);

	/** */
	static bool IsNodeEnchantable(const UK2Node & Node);

	/** */
	static UEdGraphPin * GetNextExpandablePin(
		const UK2Node & Node,
		FName           NameOfPinAfterExpandedOne
	);

#pragma endregion ~ Enchantment

#pragma region Utility

	/** */
	static void ReplaceNode(UEdGraph & Graph, UEdGraphNode & OldNode, UEdGraphNode & NewNode);

	/** */
	static void SynchronizeSplittedStructPins(const UEdGraphNode & From, UEdGraphNode & To);

	/** */
	static void CopyWildcardTypes(const UEdGraphNode & From, UEdGraphNode & To);

	/** Only works with pure blueprint editors, can't handle e.g. UMG editor */
	static FBlueprintEditor * GetActiveBlueprintEditor( );

	/** */
	static SGraphPanel * GetFocusedGraph( );

	/** @return Selected UK2Node in active graph if any, and if only one selected at a time */
	static UK2Node * GetSelectedNode(const SNodePanel & GraphWidget);
	static UK2Node * GetSelectedNode( );

	/** */
	static FName GetNextPinName(const UK2Node & Node, FName PinName);

	/** */
	template <class T>
	static ERenamePinResult HandlePinRenaming(
		T &     Target,
		FName & ExpandedPinNameRef,
		FName   OldName,
		FName   NewName,
		bool    bTest
	);

	/** */
	static void NotifyBranchEmbeddingFailed(UK2Node & Node);

#pragma endregion ~ Utility

#pragma region Node expansion

	/** */
	static UEdGraphPin * GetOriginalPin(const UK2Node & Node, FName PinName);

	/** */
	static bool IsOriginalPinPresent(const UK2Node & Node, FName PinName);

	/** */
	static void ExpandPin(UK2Node & Node, UEdGraphPin & ExpandedPin);

	/** Nicely handles simple cases. Deals with branch creation and pin reconnection */
	static void FinalizeNodeExpansion(
		FKismetCompilerContext & CompilerContext,
		UK2Node &                ExpandedNode,
		UK2Node &                ReplacingNode,
		UEdGraphPin &            BranchCondition
	);

	/** */
	static UK2Node_IfThenElse & BeginBranchConstruction(
		FKismetCompilerContext & CompilerContext,
		UK2Node &                ExpandedNode,
		UEdGraphPin &            ConditionPin
	);

	/** */
	static void TransferPinLinks(
		FKismetCompilerContext & CompilerContext,
		UK2Node &                ExpandedNode,
		UK2Node &                ReplacingNode
	);

#pragma endregion ~ Node expansion

#pragma region Branch name handling

	static bool  IsGenericPinName(FName PinName);
	static FName MakeTrueBranchName( );
	static FName MakeFalseBranchName( );
	static FName MakeTrueBranchName(FName PinName);
	static FName MakeFalseBranchName(FName PinName);

#pragma endregion ~ Branch name handling

#pragma region BPLibrary debugger attachment

	static FDelegateHandle OnBlueprintEditorOpenedHandle;

	static void OnBlueprintEditorOpened(EBlueprintType BlueprintType);

#pragma endregion ~ BPLibrary debugger attachment

private:
	/** */
	static FText GetSplittingTransactionDescription( );

private:
	/** Inputt stuff. Initialized on startup. */
	static TSharedPtr<FGraphEnchantments> InputProcessor;
	static TSharedPtr<FUICommandList>     GraphCommands;

	/** Ordered array of enchanted node's CDOs. Initialized on startup. */
	static TArray<const IEnchantedNode *> Enchantments;

	/** Menu extender. Initialized on startup. */
	static FDelegateHandle ExtendMenusHandle;



	/** Handle of temporary delegate, used while replacing node */
	static FDelegateHandle OnReplacingNodeCreatedHandle;

	/**
	 * Node to be replaced
	 *
	 * It may be valid when operation started but was cancelled
	 * But that's not a disaster, since only one operation of this type can be running at a time
	 */
	static UK2Node * NodeToReplace;



	static TSharedPtr<SNotificationItem> BranchEmbeddingFailureNotification;
};


// Template implementations //
/*************************************************************************************************/

template <class T>
ERenamePinResult FGraphEnchantments::HandlePinRenaming(
	T &     Target,
	FName & ExpandedPinNameRef,
	FName   OldName,
	FName   NewName,
	bool    bTest
)
{
	if (OldName != ExpandedPinNameRef)
		return Target.Super::RenameUserDefinedPinImpl(OldName, NewName, bTest);

	if (!bTest)
	{
		UEdGraphPin & TruePin = *Target.FindPinChecked(
			FGraphEnchantments::MakeTrueBranchName(ExpandedPinNameRef),
			EGPD_Output
		);
		UEdGraphPin & FalsePin = *Target.FindPinChecked(
			FGraphEnchantments::MakeFalseBranchName(ExpandedPinNameRef),
			EGPD_Output
		);

		TruePin.Modify( );
		FalsePin.Modify( );

		TruePin.PinName  = FGraphEnchantments::MakeTrueBranchName(NewName);
		FalsePin.PinName = FGraphEnchantments::MakeFalseBranchName(NewName);

		// Don't really know why, but its the original code
		// Probably just for notifications
		if (!TruePin.DefaultTextValue.IsEmpty( ))
		{
			TruePin.GetSchema( )->TrySetDefaultText(TruePin, TruePin.DefaultTextValue);
		}
		if (!FalsePin.DefaultTextValue.IsEmpty( ))
		{
			FalsePin.GetSchema( )->TrySetDefaultText(FalsePin, FalsePin.DefaultTextValue);
		}

		ExpandedPinNameRef = NewName;
	}

	return ERenamePinResult_Success;
}
