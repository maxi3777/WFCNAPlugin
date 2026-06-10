// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WFCCellSelector.generated.h"


class FAsyncGenerator;
class FAsyncCellSelector;

/**
 * Handles the selection of cells to collapse next when running a WFC generator.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class WFCNA_API UWFCCellSelector : public UObject
{
	GENERATED_BODY()

public:
	/** Initialize the selector for a generator */
	virtual void Initialize();

	/** Reset the selector to its initialized state */
	virtual void Reset();

	virtual TUniquePtr<FAsyncCellSelector> CreateAsyncCellSelector(FAsyncGenerator* InAsyncGenerator);
};
