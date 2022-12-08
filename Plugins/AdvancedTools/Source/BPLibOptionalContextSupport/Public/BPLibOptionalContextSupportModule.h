#pragma once

#include "Modules/ModuleManager.h"

class UBlueprint;

class FBPLibOptionalContextSupportModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule( ) override;
	virtual void ShutdownModule( ) override;

private:
	static constexpr TCHAR Prefix = TCHAR('_');

	static UBlueprint *    CompiledBlueprint;
	static FDelegateHandle PreCompileHandler;
	static FDelegateHandle CompileHandler;

	static void NotifyOnEngineInit();
	static void OnBlueprintPreCompile(UBlueprint * Blueprint);
	static void OnBlueprintCompiled( );

	static void PatchClass(UClass & Class);
	static void PatchFunction_ContextGloballyExposed(UFunction & Function);
	static void PatchFunction_ExposeContextManually(UFunction & Function);
	static void MarkCallableWithoutWorldContext(UFunction & Function);
	static void ExposeWorldContextPin(UFunction & Function);
};
