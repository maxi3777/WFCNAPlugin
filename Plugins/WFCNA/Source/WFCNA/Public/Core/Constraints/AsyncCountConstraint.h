// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncConstraint.h"
#include "Core/WFCTypes.h"


struct FWFCCountConstraintTileGroup
{
	FWFCCountConstraintTileGroup()
		: MaxCount(0)
	{
	}

	FWFCCountConstraintTileGroup(TArray<FWFCTileId> InTileIds, int32 InMaxCount)
		: TileIds(InTileIds),
		  MaxCount(InMaxCount)
	{
	}

	TArray<FWFCTileId> TileIds;

	/** Maximum number of times this group have a tile selected */
	int32 MaxCount;
};

/**
 * 
 */
class WFCNA_API FAsyncCountConstraint :public FAsyncConstraint
{
public:
	FAsyncCountConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
		TArray<FWFCCountConstraintTileGroup> InTileGroupMaxCounts, TMap<int32, int32> InTileIdsToGroups,
		TArray<int32> InTileGroupCurrentCounts);
	virtual ~FAsyncCountConstraint() override;

	virtual void Reset() override;
	virtual void NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection) override;
	virtual bool Next() override;

protected:
	/** Array of tile ids in each tile group, and their maximum count. */
	TArray<FWFCCountConstraintTileGroup> TileGroupMaxCounts;

	/** Map of tile groups indexed by tile id for fast lookup. */
	TMap<int32, int32> TileIdsToGroups;

	/** The number of times each tile group has had a tile selected. */
	TArray<int32> TileGroupCurrentCounts;

	/** Tile groups that have reached their limit and should be removed during the next update. */
	TArray<int32> TileGroupsToBan;

	/** Tile groups have already been banned. */
	TArray<int32> BannedGroups;
};
