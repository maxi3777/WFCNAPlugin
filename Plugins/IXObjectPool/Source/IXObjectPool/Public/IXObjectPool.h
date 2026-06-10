// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FIXObjectPoolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
