// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#include "Core/WFCCellSelector.h"

#include "Core/AsyncCellSelector.h"


// UWFCCellSelector
// ----------------

void UWFCCellSelector::Initialize()
{
}

void UWFCCellSelector::Reset()
{
}

TUniquePtr<FAsyncCellSelector> UWFCCellSelector::CreateAsyncCellSelector(FAsyncGenerator* InAsyncGenerator)
{
	return MakeUnique<FAsyncCellSelector>(InAsyncGenerator);
}



