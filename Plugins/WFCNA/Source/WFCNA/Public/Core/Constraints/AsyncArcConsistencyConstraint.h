// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncConstraint.h"

/**
 * 
 */
class WFCNA_API FAsyncArcConsistencyConstraint : public FAsyncConstraint
{
public:
	FAsyncArcConsistencyConstraint(
		FAsyncGenerator* InGenerator,TSharedPtr<FAsyncGrid> InGrid,TSharedPtr<FAsyncModel> InModel,
		bool InIgnoreContradictionCells,bool InDidApplyInitialConsistency,
		TArray<TArray<TArray<FWFCTileId>>> InAllowedTiles,TArray<TArray<TArray<int32>>> InSupportCounts, TArray<FWFCCellIndexAndTileId> InBansToPropagate
		);
	virtual ~FAsyncArcConsistencyConstraint() override;

	bool bIgnoreContradictionCells;
	
	virtual void Reset() override;
	virtual void NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId) override;
	virtual bool Next() override;
	
	/** Contains the allowed list of tiles for each [TileId][Direction]. */
	TArray<TArray<TArray<FWFCTileId>>> AllowedTiles;

	/** Contains the number of supports for each [CellIndex][TileId][Direction]. */
	TArray<TArray<TArray<int32>>> SupportCounts;

	/** Cached copy of the support counts after initialization for faster resetting. */
	TArray<TArray<TArray<int32>>> DefaultSupportCounts;

	/** List of banned tiles per cell that need to be propagated in the next update. */
	TArray<FWFCCellIndexAndTileId> BansToPropagate;

	TArray<FWFCCellIndexAndTileId> DefaultBansToPropagate;

	bool bDidApplyInitialConsistency;

	bool CacheDidApplyInitialConsistency;

	/** Initialize support counts and check for contradictions. */
	void ApplyInitialConsistency();

	/** Propagate changes due to banned tiles and ensure consistency. */
	bool PropagateChanges();
};
