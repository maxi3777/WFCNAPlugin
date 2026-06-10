// Copyright maxi3777. All Rights Reserved.


#include "Core/AsyncConstraint.h"


FAsyncConstraint::FAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel)
	: Generator(InGenerator),
	  Grid(InGrid),
	  Model(InModel)
{
}

FAsyncConstraint::~FAsyncConstraint()
{
}

void FAsyncConstraint::Reset()
{
}

void FAsyncConstraint::NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection)
{
}

void FAsyncConstraint::NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId)
{
}

bool FAsyncConstraint::Next()
{
	return false;
}
