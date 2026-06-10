// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Constraints/AsyncFixedTileConstraint.h"

#include "Core/AsyncGenerator.h"


FAsyncFixedTileConstraint::FAsyncFixedTileConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
	TArray<FWFCFixedTileConstraintEntry> InFixedTileMappings, bool InDidApplyInitialConstraint)
	: FAsyncConstraint(InGenerator, InGrid, InModel),
	  FixedTileMappings(InFixedTileMappings),
	  bDidApplyInitialConstraint(InDidApplyInitialConstraint), CacheDidApplyInitialConstraint(InDidApplyInitialConstraint)
{
}

FAsyncFixedTileConstraint::~FAsyncFixedTileConstraint()
{
}

void FAsyncFixedTileConstraint::Reset()
{
	FAsyncConstraint::Reset();

	bDidApplyInitialConstraint = CacheDidApplyInitialConstraint;
}

bool FAsyncFixedTileConstraint::Next()
{
	if (bDidApplyInitialConstraint)
	{
		// already applied the constraint
		return false;
	}

	bool bDidMakeChanges = false;

	// select the fixed tiles
	for (const FWFCFixedTileConstraintEntry& TileMapping : FixedTileMappings)
	{
		Generator->Select(TileMapping.CellIndex, TileMapping.TileId);
		bDidMakeChanges = true;
	}

	bDidApplyInitialConstraint = true;
	return bDidMakeChanges;
}