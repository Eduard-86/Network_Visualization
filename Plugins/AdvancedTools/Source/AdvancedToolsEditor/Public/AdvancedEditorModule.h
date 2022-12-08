#pragma once

#include "Modules/ModuleManager.h"

class FAdvancedUtilitiesEditorModule : public IModuleInterface
{
public:
	FORCEINLINE
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
