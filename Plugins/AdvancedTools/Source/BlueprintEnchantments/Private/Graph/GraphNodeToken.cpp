#include "GraphNodeToken.h"

#include "AssumeChecked.h"
#include "BlueprintEditor.h"
#include "BlueprintEditorModule.h"
#include "Kismet2/BlueprintEditorUtils.h"

TSharedRef<FGraphNodeToken> FGraphNodeToken::Create(UEdGraphNode & Node)
{
	return MakeShared<FGraphNodeToken>(Node);
}

FGraphNodeToken::FGraphNodeToken(UEdGraphNode & Node)
	: Node(&Node)
{
	FString Link;
	{
		const UBlueprint * Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Node.GetGraph( ));
		if (Blueprint)
		{
			Link = Blueprint->GetName( ) + ".";
		}

		Link += Node.GetGraph( )->GetName( ) + "." + Node.GetNodeTitle(ENodeTitleType::ListView).ToString( );
	}

	CachedText            = FText::FromString(Link);
	MessageTokenActivated = FOnMessageTokenActivated::CreateRaw(this, &FGraphNodeToken::OnActivated);
}

void FGraphNodeToken::OnActivated(const TSharedRef<IMessageToken> & Token) const
{
	if (!Node.IsValid( ))
		return;

	bool bCanBeAccessed;
#if ENGINE_MAJOR_VERSION >= 5
	bCanBeAccessed = IsValid(Node.Get( ));
#else
	bCanBeAccessed = Node->IsPendingKill();
#endif
	if (!bCanBeAccessed)
		return;

	const UEdGraph * Graph = Node->GetGraph( );
	assumeChecked(Graph);

	// Could be already removed
	if (!Graph->Nodes.Contains(Node.Get( )))
		return;

	UBlueprint * Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Graph);
	if (!Blueprint)
		return;

	// Search for on open editor
	FBlueprintEditorModule & Kismet  = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	const auto               Editors = Kismet.GetBlueprintEditors( );
	for (const auto & EditorPtr : Editors)
	{
		// Would be a nice idea to check exact type somehow
		FBlueprintEditor & Editor = static_cast<FBlueprintEditor &>(EditorPtr.Get( ));
		if (Editor.GetBlueprintObj( ) == Blueprint)
		{
			Editor.JumpToNode(Node.Get( ), false);
			Editor.AddToSelection(Node.Get( ));
			return;
		}
	}

	// Open new editor
	Kismet.CreateBlueprintEditor(EToolkitMode::Standalone, TSharedPtr<IToolkitHost>( ), Blueprint);
}
