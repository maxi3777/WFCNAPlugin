// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Constraints/AsyncArcConsistencyConstraint.h"

#include "Core/AsyncGenerator.h"
#include "Core/AsyncGrid.h"
#include "Core/AsyncModel.h"


FAsyncArcConsistencyConstraint::FAsyncArcConsistencyConstraint(
	FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
	bool InIgnoreContradictionCells, bool InDidApplyInitialConsistency,
	TArray<TArray<TArray<FWFCTileId>>> InAllowedTiles,TArray<TArray<TArray<int32>>> InSupportCounts, TArray<FWFCCellIndexAndTileId> InBansToPropagate)
	: FAsyncConstraint(InGenerator, InGrid, InModel),
	  bIgnoreContradictionCells(InIgnoreContradictionCells),
	  AllowedTiles(InAllowedTiles),
	  SupportCounts(InSupportCounts),
	  DefaultSupportCounts(InSupportCounts),
	  BansToPropagate(InBansToPropagate),
	  DefaultBansToPropagate(InBansToPropagate),
	  bDidApplyInitialConsistency(InDidApplyInitialConsistency), CacheDidApplyInitialConsistency(InDidApplyInitialConsistency)
{
}

FAsyncArcConsistencyConstraint::~FAsyncArcConsistencyConstraint()
{
}

void FAsyncArcConsistencyConstraint::Reset()
{
	//Reset parent
	FAsyncConstraint::Reset();
	
	bDidApplyInitialConsistency = CacheDidApplyInitialConsistency;
	SupportCounts = DefaultSupportCounts;
	BansToPropagate = DefaultBansToPropagate;
}

void FAsyncArcConsistencyConstraint::NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId)
{
	// update support counts
	for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
	{
		SupportCounts[CellIndex][BannedTileId][Direction] -= 1;
	}

	BansToPropagate.Push(FWFCCellIndexAndTileId(CellIndex, BannedTileId));
}

bool FAsyncArcConsistencyConstraint::Next()
{
	// check all cells and ban tile candidates to reach consistency
	if (!bDidApplyInitialConsistency)
	{
		ApplyInitialConsistency();
		bDidApplyInitialConsistency = true;
	}

	const bool bDidMakeChanges = PropagateChanges();

	return bDidMakeChanges;
}

void FAsyncArcConsistencyConstraint::ApplyInitialConsistency()
{
	// initialize support counts for each [CellIndex][TileId][Direction]
	for (int32 CellIndex = 0; CellIndex < Grid->GetNumCells(); ++CellIndex)
	{
		FWFCCell& Cell = Generator->GetCell(CellIndex);

		for (int32 TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
		{
			for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
			{
				const int32 NeighborCellIndex = Grid->GetCellIndexInDirection(CellIndex, Direction);
				if (!Grid->IsValidCellIndex(NeighborCellIndex))
				{
					continue;
				}

				// support count is the number of compatible tile ids that exist in a
				// direction from one cell to another, for a specific tile id.
				const int32 SupportCount = AllowedTiles[TileId][Direction].Num();
				SupportCounts[CellIndex][TileId][Direction] = SupportCount;
				if (SupportCount == 0 && Cell.TileCandidates.Contains(TileId))
				{
					if (Generator->Ban(CellIndex, TileId) && !bIgnoreContradictionCells)
					{
						return;
					}
					break;
				}
			}
		}
	}
}

bool FAsyncArcConsistencyConstraint::PropagateChanges()
{
	bool bDidAnyWork = false;
	while (!BansToPropagate.IsEmpty())
	{
		bDidAnyWork = true;
		const FWFCCellIndexAndTileId BanToPropagate = BansToPropagate.Pop();

		// update cells in each direction around the affected cell
		for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
		{
			const FWFCCellIndex NeighborCellIndex = Grid->GetCellIndexInDirection(BanToPropagate.CellIndex, Direction);
			if (!Grid->IsValidCellIndex(NeighborCellIndex))
			{
				continue;
			}

			const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);

			// use the outgoing direction from the banned tile to determine which tile id's were supported,
			// then decrease the support count for each one.
			const TArray<FWFCTileId>& SupportedTiles = AllowedTiles[BanToPropagate.TileId][Direction];
			for (const FWFCTileId& SupportedTileId : SupportedTiles)
			{
				// Decrement the support count for the supported tile. 
				// e.g. if tile 1 can have tile 2, 3, or 4 next to it in Direction, it starts with 3 supports.
				// when tile 3 is banned from the neighbor cell, it loses a support, if all are lost then
				// tile 1 is no longer a valid candidate.
				const int32 SupportCount = --SupportCounts[NeighborCellIndex][SupportedTileId][InvDirection];
				if (SupportCount == 0)
				{
					// no more supports left, ban this tile id for the neighbor
					if (Generator->Ban(NeighborCellIndex, SupportedTileId) && !bIgnoreContradictionCells)
					{
						// contradiction
						return true;
					}
				}
			}
		}
	}
	return bDidAnyWork;
}