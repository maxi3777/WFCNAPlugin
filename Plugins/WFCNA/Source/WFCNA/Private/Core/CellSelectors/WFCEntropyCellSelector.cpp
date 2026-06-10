// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026


#include "Core/CellSelectors/WFCEntropyCellSelector.h"

#include "Core/CellSelectors/AsyncEntropyCellSelector.h"


UWFCEntropyCellSelector::UWFCEntropyCellSelector()
	: RandomDeviation(0.001f)
{
}

TUniquePtr<FAsyncCellSelector> UWFCEntropyCellSelector::CreateAsyncCellSelector(FAsyncGenerator* InAsyncGenerator)
{
	return MakeUnique<FAsyncEntropyCellSelector>(InAsyncGenerator, RandomDeviation);
}



