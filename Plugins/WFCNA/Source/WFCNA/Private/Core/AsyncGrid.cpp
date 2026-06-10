// Copyright maxi3777. All Rights Reserved.


#include "Core/AsyncGrid.h"


FAsyncGrid::FAsyncGrid()
{
}

FAsyncGrid::~FAsyncGrid()
{
}

FWFCGridDirection FAsyncGrid::GetOppositeDirection(FWFCGridDirection Direction) const
{
	unimplemented();
	return INDEX_NONE;
}

FWFCCellIndex FAsyncGrid::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
{
	unimplemented();
	return INDEX_NONE;
}

FIntVector FAsyncGrid::GetDirectionVector(int32 Direction) const
{
	return FIntVector::ZeroValue;
}