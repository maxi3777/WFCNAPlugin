// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCTileAsset.h"
#include "Core/WFCConstraint.h"
#include "WFCBoundaryConstraint.generated.h"


UCLASS()
class WFCNA_API UWFCBoundaryConstraintSnapshot : public UWFCConstraintSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bDidApplyInitialConstraint;
};


/**
 * Require tiles placed next to the grid boundary to follow certain rules.
 * This is useful for enforcing that large tiles remain wholly inside the grid.
 */
UCLASS(DisplayName = "Boundary Constraint")
class WFCNA_API UWFCBoundaryConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	/** An optional tag query that tile edges must match to be allowed to be adjacent to the boundary. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTagQuery EdgeTypeQuery;

	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;
	virtual UWFCConstraintSnapshot* CreateSnapshot(UObject* Outer) const override;
	virtual void ApplySnapshot(const UWFCConstraintSnapshot* Snapshot) override;

	/**
	 * Add a mapping that prohibits a tile from being placed next to a grid boundary for an outgoing direction.
	 */
	void AddProhibitedAdjacentBoundaryMapping(FWFCTileId TileId, FWFCGridDirection Direction);

	/** Return true if a tile is allowed to be next to a boundary in the given direction. */
	bool CanTileBeNextToBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction) const;

	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;

protected:
	bool bIsInitialized;
	
	/** Map of tiles and the outgoing directions for which they are prohibited from being adjacent to the grid boundary. */
	TMap<FWFCTileId, TArray<FWFCGridDirection>> TileBoundaryProhibitionMap;

	bool bDidApplyInitialConstraint;
};
