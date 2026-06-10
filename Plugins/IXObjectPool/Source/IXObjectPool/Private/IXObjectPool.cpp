// Copyright maxi3777. All Rights Reserved.

#include "IXObjectPool.h"

#define LOCTEXT_NAMESPACE "FIXObjectPoolModule"

void FIXObjectPoolModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FIXObjectPoolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FIXObjectPoolModule, IXObjectPool)