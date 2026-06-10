// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncCellSelector.h"
/**
 * 
 */
class WFCNA_API FAsyncEntropyCellSelector : public FAsyncCellSelector
{
public:
	FAsyncEntropyCellSelector(FAsyncGenerator* InAsyncGenerator, float InRandomDeviation);
	virtual ~FAsyncEntropyCellSelector() override;

	float RandomDeviation;
	FRandomStream ThreadRandomStream;
	
	virtual FWFCCellIndex SelectNextCell() override;

protected:
	virtual float CalculateShannonEntropy(const FWFCCell& Cell) const;
};
