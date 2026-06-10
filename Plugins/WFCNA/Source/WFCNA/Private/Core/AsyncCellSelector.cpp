// Copyright maxi3777. All Rights Reserved.


#include "Core/AsyncCellSelector.h"


FAsyncCellSelector::FAsyncCellSelector(FAsyncGenerator* InAsyncGenerator) : AsyncGenerator(InAsyncGenerator)
{
}

FAsyncCellSelector::~FAsyncCellSelector()
{
}

FWFCCellIndex FAsyncCellSelector::SelectNextCell()
{
	return INDEX_NONE;
}
