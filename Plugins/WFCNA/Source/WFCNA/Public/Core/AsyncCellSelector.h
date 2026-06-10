// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"


class FAsyncGenerator;
/**
 * 
 */
class WFCNA_API FAsyncCellSelector
{
public:
	explicit FAsyncCellSelector(FAsyncGenerator* InAsyncGenerator);
	virtual ~FAsyncCellSelector();

	/** Select and return the next best cell to collapse. */
	virtual FWFCCellIndex SelectNextCell();

protected:
	
	FAsyncGenerator* AsyncGenerator;
};
