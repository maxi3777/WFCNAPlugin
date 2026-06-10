// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#pragma once

#include "CoreMinimal.h"
#include "AsyncCountConstraint.h"
#include "GameplayTagContainer.h"
#include "Core/WFCConstraint.h"
#include "WFCCountConstraint.generated.h"

class UWFCTileAsset;


/**
 * Limits the number of times a tile can be selected.
 */
UCLASS(Abstract)
class WFCNA_API UWFCCountConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;
	
	/** Set the maximum number of times that a set of tiles can be used. */
	void AddTileGroupMaxCountMapping(const TArray<FWFCTileId>& TileIds, int32 MaxCount);

	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;

protected:
	/** Array of tile ids in each tile group, and their maximum count. */
	TArray<FWFCCountConstraintTileGroup> TileGroupMaxCounts;

	/** Map of tile groups indexed by tile id for fast lookup. */
	TMap<int32, int32> TileIdsToGroups;

	/** The number of times each tile group has had a tile selected. */
	TArray<int32> TileGroupCurrentCounts;
};


/** Defines a max count that should be applied to all tiles with a matching tag. */
USTRUCT(BlueprintType)
struct FWFCTileTagMaxCount
{
	GENERATED_BODY()

	FWFCTileTagMaxCount()
		: MaxCount(1)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "0", UIMax = "10"))
	int32 MaxCount;
};


/**
 * Limit the max count of tiles based on the tags of each tile asset.
 */
UCLASS(Abstract, DisplayName = "Tag Count Constraint")
class WFCNA_API UWFCTagCountConstraint : public UWFCCountConstraint
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{Tag} = {MaxCount}"), Category = "TagMaxCounts")
	TArray<FWFCTileTagMaxCount> MaxCounts;

	virtual int32 GetTileMaxCount(const UWFCTileAsset* TileAsset) const;

	virtual void Initialize(UWFCGenerator* InGenerator) override;

	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;
};

