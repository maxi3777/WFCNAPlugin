// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncFixedTileConstraint.h"
#include "WFCTileAsset3D.h"
#include "WFCTileAsset3DHex.h"
#include "Core/WFCConstraint.h"
#include "WFCFixedTileConstraint.generated.h"



/**
 * A constraint that specifies exact tiles that must be used for specific cells.
 */
UCLASS(Abstract)
class WFCNA_API UWFCFixedTileConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;

	/** Add a tile constraint to be applied next time this constraint runs. */
	void AddFixedTileMapping(FWFCCellIndex CellIndex, FWFCTileId TileId);

	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;

protected:
	TArray<FWFCFixedTileConstraintEntry> FixedTileMappings;

	bool bDidApplyInitialConstraint;
};


// 3D Fixed Tile Constraints
// -------------------------

USTRUCT(BlueprintType)
struct FWFCFixedTileConstraint3DEntry
{
	GENERATED_BODY()

	FWFCFixedTileConstraint3DEntry()
		: CellLocation(FIntVector::ZeroValue),
		  TileAsset(nullptr),
		  TileRotation(0)
	{
	}

	/** The location of the cell to constrain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector CellLocation;

	/** The tile asset containing the tile to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileAsset3D> TileAsset;

	/** The rotation of the tile to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileRotation;
};


UCLASS(Abstract)
class WFCNA_API UWFCFixedTile3DConstraint : public UWFCFixedTileConstraint
{
	GENERATED_BODY()

public:
	/** The specific tiles to place and their locations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FWFCFixedTileConstraint3DEntry> FixedTiles;

	virtual void Initialize(UWFCGenerator* InGenerator) override;

	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;
};


// 3DHex Fixed Tile Constraints
// -------------------------

USTRUCT(BlueprintType)
struct FWFCFixedTileConstraint3DHexEntry
{
	GENERATED_BODY()

	FWFCFixedTileConstraint3DHexEntry()
		: CellLocation(FIntVector::ZeroValue),
		  TileAsset(nullptr),
		  TileRotation(0)
	{
	}

	/** The location of the cell to constrain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector CellLocation;

	/** The tile asset containing the tile to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileAsset3DHex> TileAsset;

	/** The rotation of the tile to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileRotation;
};

UCLASS(Abstract)
class WFCNA_API UWFCFixedTile3DHexConstraint : public UWFCFixedTileConstraint
{
	GENERATED_BODY()

public:
	/** The specific tiles to place and their locations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FWFCFixedTileConstraint3DHexEntry> FixedTiles;

	virtual void Initialize(UWFCGenerator* InGenerator) override;

	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;
};