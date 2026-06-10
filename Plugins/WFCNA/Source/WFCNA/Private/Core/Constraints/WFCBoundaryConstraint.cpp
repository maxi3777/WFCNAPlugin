// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026


#include "Core/Constraints/WFCBoundaryConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/AsyncBoundaryConstraint.h"
#include "Stats/StatsMisc.h"


void UWFCBoundaryConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	if (bIsInitialized)
	{
		return;
	}

	bDidApplyInitialConstraint = false;

	const int32 NumDirections = Grid->GetNumDirections();

	for (FWFCTileId TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		const FWFCModelAssetTile& Tile = Model->GetTileRef<FWFCModelAssetTile>(TileId);

		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			if (!CanTileBeNextToBoundary(Tile, Direction))
			{
				AddProhibitedAdjacentBoundaryMapping(TileId, Direction);
			}
		}
	}

	bIsInitialized = true;
}

void UWFCBoundaryConstraint::Reset()
{
	Super::Reset();

	bDidApplyInitialConstraint = false;
}

void UWFCBoundaryConstraint::AddProhibitedAdjacentBoundaryMapping(FWFCTileId TileId, FWFCGridDirection Direction)
{
	TileBoundaryProhibitionMap.FindOrAdd(TileId).AddUnique(Direction);
}

bool UWFCBoundaryConstraint::CanTileBeNextToBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction) const
{
	const UWFCTileAsset* TileAsset = Tile.TileAsset.Get();
	if (!TileAsset)
	{
		return false;
	}

	// convert grid direction to local space for checking edge types within the tile asset
	const FWFCGridDirection LocalDirection = Grid->InverseRotateDirection(Direction, Tile.Rotation);

	if (TileAsset->IsInteriorEdge(Tile.TileDefIndex, LocalDirection))
	{
		// for large tiles, only exterior edges can be against the boundary
		return false;
	}

	// check edge type
	const FGameplayTag EdgeType = TileAsset->GetTileDefEdgeType(Tile.TileDefIndex, LocalDirection);
	const FGameplayTagContainer EdgeTypeTags(EdgeType);
	if (!EdgeTypeQuery.IsEmpty() && !EdgeTypeQuery.Matches(EdgeTypeTags))
	{
		return false;
	}

	return true;
}

UWFCConstraintSnapshot* UWFCBoundaryConstraint::CreateSnapshot(UObject* Outer) const
{
	UWFCBoundaryConstraintSnapshot* Snapshot = NewObject<UWFCBoundaryConstraintSnapshot>(Outer);
	if (FAsyncBoundaryConstraint* AsyncArcConsistencyConstraint = static_cast<FAsyncBoundaryConstraint*>(AsyncConstraintForSnapshot))
	{
		Snapshot->bDidApplyInitialConstraint = AsyncArcConsistencyConstraint->bDidApplyInitialConstraint;
		return Snapshot;
	}
	UE_LOG(LogWFC, Error, TEXT("AsyncConstraintForSnapshot(UWFCBoundaryConstraint) not exist!"));
	return nullptr;
}

void UWFCBoundaryConstraint::ApplySnapshot(const UWFCConstraintSnapshot* Snapshot)
{
	const UWFCBoundaryConstraintSnapshot* BoundarySnapshot = Cast<UWFCBoundaryConstraintSnapshot>(Snapshot);
	bDidApplyInitialConstraint = BoundarySnapshot->bDidApplyInitialConstraint;
}

TUniquePtr<FAsyncConstraint> UWFCBoundaryConstraint::CreateAsyncConstraint(FAsyncGenerator* InGenerator,
	TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel)
{
	return MakeUnique<FAsyncBoundaryConstraint>(InGenerator, InGrid, InModel, TileBoundaryProhibitionMap, bDidApplyInitialConstraint);
}