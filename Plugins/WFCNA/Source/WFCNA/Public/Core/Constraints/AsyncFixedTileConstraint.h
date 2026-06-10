// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncConstraint.h"


struct FWFCFixedTileConstraintEntry
{
	FWFCFixedTileConstraintEntry()
		: CellIndex(INDEX_NONE),
		  TileId(INDEX_NONE)
	{
	}

	FWFCFixedTileConstraintEntry(FWFCCellIndex InCellIndex, FWFCTileId InTileId)
		: CellIndex(InCellIndex),
		  TileId(InTileId)
	{
	}

	FWFCCellIndex CellIndex;
	FWFCTileId TileId;
};
/**
 * 
 */
class WFCNA_API FAsyncFixedTileConstraint : public FAsyncConstraint
{
public:
	FAsyncFixedTileConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
		TArray<FWFCFixedTileConstraintEntry> InFixedTileMappings, bool InbDidApplyInitialConstraint);
	virtual ~FAsyncFixedTileConstraint() override;

	virtual void Reset() override;
	virtual bool Next() override;

protected:
	TArray<FWFCFixedTileConstraintEntry> FixedTileMappings;

	bool bDidApplyInitialConstraint;

	bool CacheDidApplyInitialConstraint;
};
