// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Stats/Stats.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWFC, Log, All);

DECLARE_STATS_GROUP(TEXT("WFC"), STATGROUP_WFC, STATCAT_Advanced);


class FWFCModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};