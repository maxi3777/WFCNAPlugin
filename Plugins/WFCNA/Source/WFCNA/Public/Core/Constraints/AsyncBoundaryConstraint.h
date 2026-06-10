// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncConstraint.h"

/**
 * 
 */
class WFCNA_API FAsyncBoundaryConstraint : public FAsyncConstraint
{
public:
	FAsyncBoundaryConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
		TMap<FWFCTileId,TArray<FWFCGridDirection>> InTileBoundaryProhibitionMap, bool InDidApplyInitialConstraint);
	virtual ~FAsyncBoundaryConstraint() override;

	virtual void Reset() override;
	virtual bool Next() override;
	
	/** Map of tiles and the outgoing directions for which they are prohibited from being adjacent to the grid boundary. */
	TMap<FWFCTileId, TArray<FWFCGridDirection>> TileBoundaryProhibitionMap;

	bool bDidApplyInitialConstraint;

	bool CacheDidApplyInitialConstraint;

	/** Cached map of tiles to ban for each cell. Calculated after the first time this constraint is run in case it needs to re-run */
	TMap<FWFCCellIndex, TArray<FWFCTileId>> TilesToBan;

	/** Return true if a tile is not allowed to be adjacent to boundaries in the given outgoing directions. */
	bool IsTileBoundaryDirectionProhibited(FWFCTileId TileId, const TArray<FWFCGridDirection>& BoundaryDirections) const;
};
