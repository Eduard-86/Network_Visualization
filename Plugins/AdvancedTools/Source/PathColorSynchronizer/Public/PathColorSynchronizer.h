#pragma once

#include "Modules/ModuleManager.h"

class FPathColorSynchronizer : public IModuleInterface
{
public:

	/** IModuleInterface implementation */

	virtual void ShutdownModule() override;
};
