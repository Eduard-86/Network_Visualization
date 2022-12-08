#include "BlueprintEnchantmentsModule.h"

#include "AssumeChecked.h"
#include "BlueprintEditor.h"
#include "BlueprintEditorModule.h"
#include "EdGraphUtilities.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "SKismetInspector.h"
#include "BlueprintGraph/Classes/K2Node_EditablePinBase.h"
#include "Graph/GraphEnchantments.h"
#include "GraphEditor/Public/GraphEditorModule.h"
#include "Inspector/KismetInspectorEnchantments.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetReinstanceUtilities.h"
#include "NodeWidget/AdvancedK2NodeFactory.h"
#include "PropertyEditor/Private/SDetailsViewBase.h"

TArray<UBlueprint *> FBlueprintEnchantmentsModule::CompilationQueue;

FDelegateHandle FBlueprintEnchantmentsModule::OnRegisterTabsForEditorHandler;
FDelegateHandle FBlueprintEnchantmentsModule::OnBlueprintPreCompileHandler;
FDelegateHandle FBlueprintEnchantmentsModule::OnBlueprintCompiledHandler;

TSharedPtr<FEnchantedK2NodeFactory> FBlueprintEnchantmentsModule::EnchantedK2NodeFactory;

#define LOCTEXT_NAMESPACE "BlueprintEnchantments"

//Keep in sync with AnimGraph/Public/IAnimBlueprintCompilationContext.h
#define ANIM_FUNC_DECORATOR	TEXT("__AnimFunc")

//  //
/*************************************************************************************************/

class SDetailsViewBase_Exposed : public SDetailsViewBase
{
public:
	using SDetailsViewBase::InstancedClassToDetailLayoutMap;
};


//  //
/*************************************************************************************************/

void FBlueprintEnchantmentsModule::StartupModule( )
{
	if (!GEditor)
		return;

	{
		FBlueprintEditorModule & Kismet = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");

		if (!IsRunningCommandlet( ))
		{
			OnRegisterTabsForEditorHandler = Kismet.OnRegisterTabsForEditor( ).AddStatic(
				&FBlueprintEnchantmentsModule::OnRegisterTabsForEditor
			);
		}

		// TODO Consider using FBlueprintCompilationManager::RegisterCompilerExtension instead
		// it provides a lot more access to  and control of what's going on
		// And provides theoretical possibility to remove __WorldContext pin from already generated function layouts
		// Though this will not help against the fact, that graph nodes are created BEFORE blueprint is compiled
		OnBlueprintPreCompileHandler = GEditor->OnBlueprintPreCompile( ).AddStatic(
			&FBlueprintEnchantmentsModule::OnBlueprintPreCompile
		);
		OnBlueprintCompiledHandler = GEditor->OnBlueprintCompiled( ).AddStatic(
			&FBlueprintEnchantmentsModule::OnBlueprintCompiled
		);
	}


	{
		EnchantedK2NodeFactory = MakeShared<FEnchantedK2NodeFactory>( );
		FEdGraphUtilities::RegisterVisualNodeFactory(EnchantedK2NodeFactory);
	}


	FGraphEnchantments::StartUp( );
}

void FBlueprintEnchantmentsModule::ShutdownModule( )
{
	if (!GEditor)
		return;

	{
		FBlueprintEditorModule & Kismet = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");

		Kismet.OnRegisterTabsForEditor( ).Remove(OnRegisterTabsForEditorHandler);
		GEditor->OnBlueprintPreCompile( ).Remove(OnBlueprintPreCompileHandler);
		GEditor->OnBlueprintCompiled( ).Remove(OnBlueprintCompiledHandler);
	}

	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(EnchantedK2NodeFactory);
	}

	FGraphEnchantments::ShutDown( );
}


//  //
/*************************************************************************************************/

void FBlueprintEnchantmentsModule::OnRegisterTabsForEditor(
	FWorkflowAllowedTabSet &     WorkflowAllowedTabSet,
	FName                        ModeName,
	TSharedPtr<FBlueprintEditor> BlueprintEditor
)
{
	assumeChecked(BlueprintEditor.IsValid());
	IDetailsView * PropertyView = BlueprintEditor->GetInspector( )->GetPropertyView( ).Get( );
	assumeChecked(PropertyView);

	Inject<UEdGraph>(*PropertyView);
	Inject<UK2Node_EditablePinBase>(*PropertyView);
	Inject<UK2Node_CallFunction>(*PropertyView);
}

void FBlueprintEnchantmentsModule::Inject(
	IDetailsView & DetailsView,
	UClass *       Class
)
{
	FDetailLayoutCallback * OriginalCallbackWrapper = ((SDetailsViewBase_Exposed &)DetailsView)
	                                                  .InstancedClassToDetailLayoutMap.Find(Class);
	check(OriginalCallbackWrapper);
	const FOnGetDetailCustomizationInstance OriginalCallback = OriginalCallbackWrapper->DetailLayoutDelegate;
	// TODO
	// Don't want to patch what is already patched
	// But, can't really access delegate internals
	// Next does not always work:
	// check(OriginalCallback.GetBoundProgramCounterForTimerManager());
	const FOnGetDetailCustomizationInstance Hook = FOnGetDetailCustomizationInstance::CreateLambda(
		[OriginalCallback]( )-> TSharedRef<class IDetailCustomization>
		{
			return MakeShareable(new FKismetInspectorEnchantments(OriginalCallback));
		}
	);
	OriginalCallbackWrapper->DetailLayoutDelegate = Hook;
}

//  //
/*************************************************************************************************/

void FBlueprintEnchantmentsModule::OnBlueprintPreCompile(
	UBlueprint * Blueprint
)
{
	assumeChecked(Blueprint);
	// Only proceed regular blueprints
	if (Blueprint->GetClass( ) != UBlueprint::StaticClass( ))
		return;

	// Queue BP for patching after full compile
	CompilationQueue.Push(Blueprint);

	// Patch function skeletons now as it influences compilation
	for (UEdGraph * FunctionGraph : Blueprint->FunctionGraphs)
	{
		const FName FunctionName = FunctionGraph->GetFName( );
		if (FunctionName.ToString( ).EndsWith(ANIM_FUNC_DECORATOR))
			continue;

		FEnchantments Enchantments(*FunctionGraph);
		assumeChecked(Blueprint->SkeletonGeneratedClass);
		UFunction * FunctionSkeleton = Blueprint->SkeletonGeneratedClass->
		                                          FindFunctionByName(FunctionGraph->GetFName( ));
		assumeChecked(FunctionSkeleton);

		Enchantments.EnchantFunctions(FunctionSkeleton);

		// TODO Wellp. It works without it. Cause yet enchantments does not alter node body. But will it be always like this?
		// Uncommented, terrifically increases initial asset loading times; obviously works incorrect
		//
		// Repeat node refreshing after patching skeleton
		// Sadly it happens right before this event is fired, so gotta repeat it
		//RefreshEnchantedNodes(Blueprint, Blueprint->bIsRegeneratingOnLoad);
	}
}

void FBlueprintEnchantmentsModule::OnBlueprintCompiled( )
{
	if (CompilationQueue.Num( ) == 0)
		return;

	for (int i = CompilationQueue.Num( ) - 1; i >= 0; i--)
	{
		UBlueprint * Blueprint = CompilationQueue[i];

		// Not time yet
		if (Blueprint->bBeingCompiled == true)
			continue;

		// Make magic
		for (UEdGraph * FunctionGraph : Blueprint->FunctionGraphs)
		{
			// Skeleton  is already enchanted actually, buuut better safe the sorry, right?
			FEnchantments::EnchantGeneratedFunctionsStatic(*FunctionGraph);
		}

		CompilationQueue.RemoveAt(i);
	}
}

void FBlueprintEnchantmentsModule::RefreshEnchantedNodes(
	UBlueprint * Blueprint,
	const bool   bIsRegeneratingOnLoad
)
{
	//
	// Mimic the original behaviour: BlueprintCompilationManager.cpp::1076
	// Can't use TGuardValue for a bitfield
	//
	const bool OriginalFlag          = Blueprint->bIsRegeneratingOnLoad;
	Blueprint->bIsRegeneratingOnLoad = bIsRegeneratingOnLoad;

	// Some nodes are set up to do things during reconstruction only when this flag is NOT set.
	if (bIsRegeneratingOnLoad)
	{
		FBlueprintEditorUtils::ReconstructAllNodes(Blueprint);
		// Though we don't deprecate anything so don't run this one again
		//FBlueprintEditorUtils::ReplaceDeprecatedNodes(CompiledBlueprint);
	}
	else
	{
		// matching existing behavior, when compiling a BP not on load we refresh nodes
		// before compiling:
		FBlueprintCompileReinstancer::OptionallyRefreshNodes(Blueprint);
		TArray<UBlueprint *> DependentBlueprints;
		FBlueprintEditorUtils::GetDependentBlueprints(Blueprint, DependentBlueprints);

		for (UBlueprint * CurrentBP : DependentBlueprints)
		{
			const EBlueprintStatus OriginalStatus             = CurrentBP->Status;
			UPackage * const       Package                    = CurrentBP->GetOutermost( );
			const bool             bStartedWithUnsavedChanges = Package != nullptr ? Package->IsDirty( ) : true;

			FBlueprintEditorUtils::RefreshExternalBlueprintDependencyNodes(
				CurrentBP,
				Blueprint->GeneratedClass
			);

			CurrentBP->Status = OriginalStatus;
			if (Package != nullptr && Package->IsDirty( ) && !bStartedWithUnsavedChanges)
			{
				Package->SetDirtyFlag(false);
			}
		}
	}
	//
	// Reset flag back
	//
	Blueprint->bIsRegeneratingOnLoad = OriginalFlag;
}


#undef LOCTEXT_NAMESPACE
#undef ANIM_FUNC_DECORATOR

IMPLEMENT_MODULE(FBlueprintEnchantmentsModule, BlueprintEnchantments)
