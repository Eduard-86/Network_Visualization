#include "BPLibOptionalContextSupport/Public/BPLibOptionalContextSupportModule.h"

#include "Editor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "BlueprintGraph/Classes/EdGraphSchema_K2.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "BPLibInMacroLibSupport"

UBlueprint *    FBPLibOptionalContextSupportModule::CompiledBlueprint;
FDelegateHandle FBPLibOptionalContextSupportModule::PreCompileHandler;
FDelegateHandle FBPLibOptionalContextSupportModule::CompileHandler;

DECLARE_LOG_CATEGORY_CLASS(LogBPLibOptionalContextSupport, Display, VeryVerbose)

#define LogPatchingResult(Verbosity, Message, ...)\
	UE_LOG(\
		LogBPLibOptionalContextSupport,\
		Verbosity,\
		TEXT("%s::%s: ") Message,\
		*Function.GetOuterUClassUnchecked()->GetName(),\
		*Function.GetName(),\
		__VA_ARGS__\
	)


void FBPLibOptionalContextSupportModule::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	UE_LOG(LogBPLibOptionalContextSupport, Display, TEXT("Patching BP libraries on startup"));
	for (TObjectIterator<UClass> Class = TObjectIterator<UClass>(); Class; ++Class)
	{
		const bool bIsNative    = Class->IsNative();
		const bool bIsBPLibrary = Class->IsChildOf(UBlueprintFunctionLibrary::StaticClass());
		// Patch outdated classes just in case. Moreover, skeleton classes are actually used.
		const bool bIsOutdated =
			Class->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists)
			|| FKismetEditorUtilities::IsClassABlueprintSkeleton(*Class);
		if (!bIsNative && bIsBPLibrary)
		{
			PatchClass(**Class);
		}
	}

	UObject::StaticClass()->SetMetaData(FBlueprintMetadata::MD_ShowWorldContextPin, TEXT("true"));
	// UWorld::StaticClass()->SetMetaData(FBlueprintMetadata::MD_AllowableBlueprintVariableType, TEXT("true"));
	UE_LOG(
		LogBPLibOptionalContextSupport,
		Display,
		TEXT("UObject class marked with ShowWorldContextPin")
	);
	// Got to be done later then there
	//
	// FCoreDelegates::OnFEngineLoopInitComplete.AddStatic(
	// 	&FBPLibOptionalContextSupportModule::NotifyOnEngineInit
	// );

	PreCompileHandler = GEditor->OnBlueprintPreCompile().AddStatic(&OnBlueprintPreCompile);
	CompileHandler    = GEditor->OnBlueprintCompiled().AddStatic(&OnBlueprintCompiled);
}

void FBPLibOptionalContextSupportModule::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// May be already undefined
	if (GEditor)
	{
		GEditor->OnBlueprintPreCompile().Remove(PreCompileHandler);
		GEditor->OnBlueprintPreCompile().Remove(CompileHandler);
	}
}

void FBPLibOptionalContextSupportModule::NotifyOnEngineInit( )
{
	// FSlateNotificationManager::Get().AddNotification(
	// 	FNotificationInfo(
	// 		LOCTEXT(
	// 			"Object marked with ShowWorldContextPin",
	// 			"UObject patched! You can now use all functions in macros."
	// 		)
	// 	)
	// );
}

void FBPLibOptionalContextSupportModule::OnBlueprintPreCompile(UBlueprint * Blueprint)
{
	if (
		Blueprint->BlueprintType == BPTYPE_FunctionLibrary
		&& Blueprint->ParentClass == UBlueprintFunctionLibrary::StaticClass()
	)
	{
		CompiledBlueprint = Blueprint;
	}
}

void FBPLibOptionalContextSupportModule::OnBlueprintCompiled( )
{
	if (CompiledBlueprint)
	{
		UE_LOG(
			LogBPLibOptionalContextSupport,
			Display,
			TEXT("Patching BP library after compilation: %s"),
			*CompiledBlueprint->GetName()
		)
		if (CompiledBlueprint->GeneratedClass)
		{
			PatchClass(**CompiledBlueprint->GeneratedClass);
		}
		if (CompiledBlueprint->GeneratedClass)
		{
			PatchClass(**CompiledBlueprint->SkeletonGeneratedClass);
		}
		CompiledBlueprint = nullptr;
	}
}

void FBPLibOptionalContextSupportModule::PatchClass(UClass & Class)
{
	checkSlow(Class.IsChildOf(UBlueprintFunctionLibrary::StaticClass()));

	TArray<FName> FunctionNames;
	Class.GenerateFunctionList(FunctionNames);
	for (const FName & FunctionName : FunctionNames)
	{
		UFunction * Function = Class.FindFunctionByName(FunctionName);
		checkSlow(Function);

		PatchFunction_ContextGloballyExposed(*Function);
	}
}

void FBPLibOptionalContextSupportModule::PatchFunction_ContextGloballyExposed(UFunction & Function)
{
	const FString FunctionNameString = Function.GetName();
	// Don't want to assume anything from name "_"
	if (FunctionNameString.Len() < 2)
	{
		LogPatchingResult(Display, "name is too short to assume anything");
		return;
	}

	if (FunctionNameString[0] == Prefix)
	{
		MarkCallableWithoutWorldContext(Function);
	}
	else
	{
		LogPatchingResult(Display, "skipped");
	}
}

void FBPLibOptionalContextSupportModule::PatchFunction_ExposeContextManually(UFunction & Function)
{
	const FString FunctionNameString = Function.GetName();
	// Don't want to assume anything from names like "_" or "__"
	if (FunctionNameString.Len() < 3)
	{
		LogPatchingResult(Display, "name is too short to assume anything");
		return;
	}

	const int NumUnderscores =
		0
		+ (FunctionNameString[0] == Prefix)
		+ (FunctionNameString[1] == Prefix);
	switch (NumUnderscores)
	{
		case 1:
			ExposeWorldContextPin(Function);
			break;
		case 2:
			MarkCallableWithoutWorldContext(Function);
			break;
		default:
			LogPatchingResult(Display, "skipped");
			break;
	}
}

void FBPLibOptionalContextSupportModule::MarkCallableWithoutWorldContext(UFunction & Function)
{
	Function.SetMetaData(FBlueprintMetadata::MD_CallableWithoutWorldContext, TEXT(""));
	if (Function.HasMetaData(FBlueprintMetadata::MD_WorldContext))
	{
		LogPatchingResult(Display, "marked CallableWithoutWorldContext");
	}
	else
	{
		LogPatchingResult(Warning, "marked CallableWithoutWorldContext, though has no WorldContext");
	}
}

void FBPLibOptionalContextSupportModule::ExposeWorldContextPin(UFunction & Function)
{
	static const TCHAR * WorldContextParamName  = TEXT("__WorldContext");
	static const FName   WorldContextParamFName = FName(WorldContextParamName);
	for (
		FProperty * Property = static_cast<FProperty*>(Function.ChildProperties);
		Property;
		Property = static_cast<FProperty*>(Property->Next)
	)
	{
		if (Property->NamePrivate == WorldContextParamFName)
		{
			Property->PropertyFlags |= CPF_ReferenceParm;
			Function.RemoveMetaData(FBlueprintMetadata::MD_WorldContext);
			// Next 2 lines allows to use 'self' as default for WorldContext pin, but this can
			// often lead to implicit errors
			// 
			// Function.SetMetaData(FBlueprintMetadata::MD_AutoCreateRefTerm, WorldContextParamName);
			// Function.SetMetaData(FBlueprintMetadata::MD_DefaultToSelf, WorldContextParamName);

			LogPatchingResult(Display, "exposed WorldContext pin");
			return;
		}
	}

	LogPatchingResult(Display, "is marked for WorldContext pin exposure, but has no such pin");
}

#undef LOCTEXT_NAMESPACE
#undef LogPatchingResult

IMPLEMENT_MODULE(FBPLibOptionalContextSupportModule, BPLibOptionalContextSupport)
