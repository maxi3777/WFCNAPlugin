// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCArcConsistencyConstraint.h"
#include "WFCTileAsset.h"
#include "WFCEdgeConstraint.generated.h"

class UWFCAssetModel;


/**
 * Constrains tiles such that only those with matching edge tags can be placed next to each other.
 * TODO: rename to something with 'Tag' in it for clarity.
 */
UCLASS(DisplayName = "Adjacency Constraint")
class WFCNA_API UWFCEdgeConstraint : public UWFCArcConsistencyConstraint
{
	GENERATED_BODY()

public:
	UWFCEdgeConstraint();

	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void ApplySnapshot(const UWFCConstraintSnapshot* Snapshot) override;

	/** Return true if two edges are allowed to be next to each other. */
	virtual bool AreEdgesCompatible(const FGameplayTag& EdgeA, const FGameplayTag& EdgeB) const;

	/** Return true if TileB can be placed next to TileA in a direction going from A -> B. */
	virtual bool AreTilesCompatible(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB, FWFCGridDirection Direction) const;

	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;

protected:
	bool bIsInitializedFromTiles;
	
	/** Reference to the asset model required for this constraint. */
	UPROPERTY(Transient)
	TObjectPtr<const UWFCAssetModel> AssetModel;

	/**
	 * Initialize the allowed tiles by iterating all tiles in the model,
	 * looking up their edge types, and adding mappings.
	 */
	void InitializeFromTiles();
};
