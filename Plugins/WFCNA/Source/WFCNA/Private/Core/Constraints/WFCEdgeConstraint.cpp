// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Constraints/WFCEdgeConstraint.h"

#include "WFCAssetModel.h"
#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Stats/StatsMisc.h"
#include "Core/Constraints/AsyncArcConsistencyConstraint.h"


UWFCEdgeConstraint::UWFCEdgeConstraint()
	: bIsInitializedFromTiles(false)
{
}

void UWFCEdgeConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	AssetModel = Cast<UWFCAssetModel>(Model);
	if (!AssetModel)
	{
		UE_LOG(LogWFC, Error, TEXT("%s requires a UWFCAssetModel: %s"), *GetClass()->GetName(), *GetNameSafe(GetOuter()));
		return;
	}

	if (!bIsInitializedFromTiles)
	{
		InitializeFromTiles();
		bIsInitializedFromTiles = true;
	}

	LogDebugInfo();
}

void UWFCEdgeConstraint::ApplySnapshot(const UWFCConstraintSnapshot* Snapshot)
{
	Super::ApplySnapshot(Snapshot);

	bIsInitializedFromTiles = true;
}

bool UWFCEdgeConstraint::AreEdgesCompatible(const FGameplayTag& EdgeA, const FGameplayTag& EdgeB) const
{
	return EdgeA.IsValid() && EdgeB.IsValid() && EdgeA == EdgeB;
}

bool UWFCEdgeConstraint::AreTilesCompatible(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
                                            FWFCGridDirection Direction) const
{
	check(TileA.TileAsset.IsValid());
	check(TileB.TileAsset.IsValid());

	// the given direction is from A -> B, convert it to local space for checking A's edge
	const FWFCGridDirection LocalOutDirectionA = Grid->InverseRotateDirection(Direction, TileA.Rotation);

	if (TileA.TileAsset->IsInteriorEdge(TileA.TileDefIndex, LocalOutDirectionA))
	{
		// interior edge, only matching tile would be the exact neighbor of the same rotation
		return TileA.Rotation == TileB.Rotation &&
			TileA.TileAsset == TileB.TileAsset &&
			TileA.TileAsset->GetTileDefInDirection(TileA.TileDefIndex, LocalOutDirectionA) == TileB.TileDefIndex;
	}

	const FGameplayTag EdgeTypeA = TileA.TileAsset->GetTileDefEdgeType(TileA.TileDefIndex, LocalOutDirectionA);

	// flip the direction, then convert to local space for checking B's edge
	const FWFCGridDirection WorldOutDirectionB = Grid->GetOppositeDirection(Direction);
	const FWFCGridDirection LocalOutDirectionB = Grid->InverseRotateDirection(WorldOutDirectionB, TileB.Rotation);
	const FGameplayTag EdgeTypeB = TileB.TileAsset->GetTileDefEdgeType(TileB.TileDefIndex, LocalOutDirectionB);

	return AreEdgesCompatible(EdgeTypeA, EdgeTypeB);
}

void UWFCEdgeConstraint::InitializeFromTiles()
{
	const int32 NumDirections = Grid->GetNumDirections();

	// iterate over all distinct pairs of tiles, including reflectivity, comparing socket types for compatibility
	for (FWFCTileId TileIdA = 0; TileIdA <= Model->GetMaxTileId(); ++TileIdA)
	{
		const FWFCModelAssetTile& TileA = Model->GetTileRef<FWFCModelAssetTile>(TileIdA);

		// compare A <-> A for each direction
		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			if (AreTilesCompatible(TileA, TileA, Direction))
			{
				AddAllowedTileForDirection(TileIdA, Direction, TileIdA);
			}
		}

		for (FWFCTileId TileIdB = TileIdA + 1; TileIdB <= Model->GetMaxTileId(); ++TileIdB)
		{
			const FWFCModelAssetTile& TileB = Model->GetTileRef<FWFCModelAssetTile>(TileIdB);

			// compare A <-> B for each direction
			for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
			{
				if (AreTilesCompatible(TileA, TileB, Direction))
				{
					AddAllowedTileForDirection(TileIdA, Direction, TileIdB);

					// add opposite directly as well
					const FWFCGridDirection OppositeDirection = Grid->GetOppositeDirection(Direction);
					AddAllowedTileForDirection(TileIdB, OppositeDirection, TileIdA);
				}
			}
		}
	}
}

TUniquePtr<FAsyncConstraint> UWFCEdgeConstraint::CreateAsyncConstraint(FAsyncGenerator* InGenerator,
	TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel)
{
	return Super::CreateAsyncConstraint(InGenerator, InGrid, InModel);
}
