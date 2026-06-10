// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Constraints/AsyncCountConstraint.h"

#include "Core/AsyncGenerator.h"


FAsyncCountConstraint::FAsyncCountConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
		TArray<FWFCCountConstraintTileGroup> InTileGroupMaxCounts, TMap<int32, int32> InTileIdsToGroups,
		TArray<int32> InTileGroupCurrentCounts)
	: FAsyncConstraint(InGenerator, InGrid, InModel),
	  TileGroupMaxCounts(InTileGroupMaxCounts),
	  TileIdsToGroups(InTileIdsToGroups),
	  TileGroupCurrentCounts(InTileGroupCurrentCounts)
{
}

FAsyncCountConstraint::~FAsyncCountConstraint()
{
}

void FAsyncCountConstraint::Reset()
{
	FAsyncConstraint::Reset();

	TileGroupCurrentCounts.Reset(TileGroupMaxCounts.Num());
	TileGroupCurrentCounts.SetNum(TileGroupMaxCounts.Num());
	TileGroupsToBan.Reset();
	BannedGroups.Reset();
}

void FAsyncCountConstraint::NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection)
{
	if (bHasSelection)
	{
		const FWFCCell& Cell = Generator->GetCell(CellIndex);
		const FWFCTileId TileId = Cell.GetSelectedTileId();
		const int32 TileGroupIndex = TileIdsToGroups.Contains(TileId) ? TileIdsToGroups[TileId] : INDEX_NONE;
		if (TileGroupIndex != INDEX_NONE && !BannedGroups.Contains(TileGroupIndex))
		{
			const FWFCCountConstraintTileGroup& TileGroup = TileGroupMaxCounts[TileGroupIndex];
			TileGroupCurrentCounts[TileGroupIndex] += 1;
			if (TileGroupCurrentCounts[TileGroupIndex] >= TileGroup.MaxCount)
			{
				TileGroupsToBan.AddUnique(TileGroupIndex);
			}
		}
	}
}

bool FAsyncCountConstraint::Next()
{
	bool bDidMakeChanges = false;

	if (TileGroupsToBan.Num() > 0)
	{
		// accumulate all tile ids from all groups to ban
		TArray<FWFCTileId> TileIdsToBan;
		for (const int32 TileGroupIndex : TileGroupsToBan)
		{
			TileIdsToBan.Append(TileGroupMaxCounts[TileGroupIndex].TileIds);
			BannedGroups.AddUnique(TileGroupIndex);
		}
		// remove from all unselected cells
		for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
		{
			FWFCCell& Cell = Generator->GetCell(CellIndex);
			if (!Cell.HasSelection())
			{
				if (Generator->BanMultiple(CellIndex, TileIdsToBan))
				{
					// contradiction
					return true;
				}

				bDidMakeChanges = true;
			}
		}

		TileGroupsToBan.Reset();
	}
	return bDidMakeChanges;
}
