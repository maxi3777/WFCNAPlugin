// Copyright maxi3777. All Rights Reserved.


#include "Core/Constraints/AsyncBoundaryConstraint.h"

#include "Core/AsyncGenerator.h"
#include "Core/AsyncGrid.h"


FAsyncBoundaryConstraint::FAsyncBoundaryConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
	TMap<FWFCTileId,TArray<FWFCGridDirection>> InTileBoundaryProhibitionMap, bool InDidApplyInitialConstraint)
	: FAsyncConstraint(InGenerator, InGrid, InModel),
	  TileBoundaryProhibitionMap(InTileBoundaryProhibitionMap),
	  bDidApplyInitialConstraint(InDidApplyInitialConstraint), CacheDidApplyInitialConstraint(InDidApplyInitialConstraint)
{
}

FAsyncBoundaryConstraint::~FAsyncBoundaryConstraint()
{
}

void FAsyncBoundaryConstraint::Reset()
{
	FAsyncConstraint::Reset();

	bDidApplyInitialConstraint = CacheDidApplyInitialConstraint;
}

bool FAsyncBoundaryConstraint::Next()
{
	if (bDidApplyInitialConstraint)
	{
		// already applied the constraint
		return false;
	}

	bool bDidMakeChanges = false;

	// iterate all cells and ban tiles that can't be next to boundaries

	const int32 NumDirections = Grid->GetNumDirections();

	// if tiles to ban is already filled out, don't recalculate it, since it
	// will be the same each time this constraint is first run.
	if (TilesToBan.IsEmpty())
	{
		for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
		{
			const FWFCCell& CellToCheck = Generator->GetCell(CellIndex);
			if (CellToCheck.HasSelection())
			{
				// don't change cells that are already selected
				continue;
			}

			// find all boundary directions
			TArray<FWFCGridDirection> BoundaryDirections;
			for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
			{
				const FWFCCellIndex AdjacentCellIndex = Grid->GetCellIndexInDirection(CellIndex, Direction);
				if (!Grid->IsValidCellIndex(AdjacentCellIndex))
				{
					BoundaryDirections.AddUnique(Direction);
				}
			}

			if (BoundaryDirections.IsEmpty())
			{
				// cell is not next to a boundary
				continue;
			}

			TArray<FWFCTileId> TileIdsToBan;
			for (const FWFCTileId& TileId : CellToCheck.TileCandidates)
			{
				// check each tile for any prohibited boundary directions
				if (IsTileBoundaryDirectionProhibited(TileId, BoundaryDirections))
				{
					TileIdsToBan.Add(TileId);
				}
			}

			if (TileIdsToBan.Num() > 0)
			{
				TilesToBan.Add(CellIndex, TileIdsToBan);
			}
		}
	}

	// apply bans
	if (!TilesToBan.IsEmpty())
	{
		for (const auto& Elem : TilesToBan)
		{
			if (Generator->BanMultiple(Elem.Key, Elem.Value))
			{
				// contradiction
				return true;
			}
		}
		bDidMakeChanges = true;
	}
	
	bDidApplyInitialConstraint = true;
	
	return bDidMakeChanges;
}

bool FAsyncBoundaryConstraint::IsTileBoundaryDirectionProhibited(FWFCTileId TileId, const TArray<FWFCGridDirection>& BoundaryDirections) const
{
	if (const TArray<FWFCGridDirection>* ProhibitedDirectionsPtr = TileBoundaryProhibitionMap.Find(TileId))
	{
		return ProhibitedDirectionsPtr->ContainsByPredicate([BoundaryDirections](const FWFCGridDirection& Direction)
		{
			return BoundaryDirections.Contains(Direction);
		});
	}
	return false;
}