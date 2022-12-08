// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

struct FEnchantedK2NodeFactory;

class UBlueprint;
class UEdGraphPin;
class UEdGraphNode;
class UEdGraph;
class UEdGraphSchema_K2;

class FBlueprintEditor;
class IBlueprintEditor;
class FWorkflowAllowedTabSet;
class IDetailsView;

class FBlueprintEnchantmentsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule( ) override;
	virtual void ShutdownModule( ) override;
	virtual bool SupportsDynamicReloading( ) override { return false; }

#pragma region Compilation stuff

private:
	/**
	 * There's a BlueprintEditorOpened event, but it doesn't pass the opened editor, and is called BEFORE array of all
	 * open editors is updated
	 */
	static void OnRegisterTabsForEditor
	(
		FWorkflowAllowedTabSet &     WorkflowAllowedTabSet,
		FName                        ModeName,
		TSharedPtr<FBlueprintEditor> BlueprintEditor
	);

	static void Inject(IDetailsView & DetailsView, UClass * Class);

	template <class T>
	static void Inject(IDetailsView & DetailsView) { Inject(DetailsView, T::StaticClass( )); }

	static void OnBlueprintPreCompile(UBlueprint * Blueprint);
	static void OnBlueprintCompiled( );
	static void RefreshEnchantedNodes(UBlueprint * Blueprint, bool bIsRegeneratingOnLoad);


	/** Multiple blueprints can be compiled at the same time if they make a circular dependency */
	static TArray<UBlueprint*> CompilationQueue;

#pragma endregion ~ Compilation stuff

private:
	static FDelegateHandle OnRegisterTabsForEditorHandler;
	static FDelegateHandle OnBlueprintPreCompileHandler;
	static FDelegateHandle OnBlueprintCompiledHandler;

	static TSharedPtr<FEnchantedK2NodeFactory> EnchantedK2NodeFactory;
};
