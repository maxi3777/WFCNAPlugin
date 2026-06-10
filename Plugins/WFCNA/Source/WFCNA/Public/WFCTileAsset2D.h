// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCTileAsset.h"
#include "WFCTileDefType.h"
#include "WFCTileAsset2D.generated.h"


class UWFCUnit;
struct FISMBucketKey;

UENUM(BlueprintType)
enum class EWFCSquareRotationType : uint8
{
	One,   // 不旋转 (0°)
	Two,   // 两向旋转 (0°, 90°)
	Four   // 四向旋转 (0°, 90°, 180°, 270°)
};

/** The edge types for a 2D tile */
UENUM(BlueprintType)
enum class EWFCTile2DEdge : uint8
{
	XPos UMETA(DisplayName = "+X"),
	YPos UMETA(DisplayName = "+Y"),
	XNeg UMETA(DisplayName = "-X"),
	YNeg UMETA(DisplayName = "-Y"),
};


/**
 * Definition of a tile within a group.
 */
USTRUCT(BlueprintType)
struct FWFCTileDef2D
{
	GENERATED_BODY()

	FWFCTileDef2D()
		: Location(FIntPoint::ZeroValue)
	{
		EdgeTypes = {
			{EWFCTile2DEdge::XPos, FGameplayTag::EmptyTag},
			{EWFCTile2DEdge::YPos, FGameplayTag::EmptyTag},
			{EWFCTile2DEdge::XNeg, FGameplayTag::EmptyTag},
			{EWFCTile2DEdge::YNeg, FGameplayTag::EmptyTag},
		};
	}

	/** The relative location of this tile within the group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Location;

	/** The actors to spawn for this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FActorInfoEntry> ActorsInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStaticMeshInfoEntry> StaticMeshesInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UWFCUnit> WFCUnit;

	/** The socket types for all edges of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"))
	TMap<EWFCTile2DEdge, FGameplayTag> EdgeTypes;
};


/**
 * A 2D tile with a socket type on each side for use with adjacency constraints.
 * References an actor to be spawned for each tile.
 */
UCLASS()
class WFCNA_API UWFCTileAsset2D : public UWFCTileAsset
{
	GENERATED_BODY()

public:
	UWFCTileAsset2D();

	/**
	 * The dimensions of this tile. Tile assets can contain multiple actual tiles
	 * which will have fixed adjacency rules setup for them.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Dimensions;

	/** How this piece be rotating? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWFCSquareRotationType RotationType;

	/** The definitions for each tile within this asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "Location"))
	TArray<FWFCTileDef2D> TileDefs;

	/** Return a tile def by location, as well as its index. */
	UFUNCTION(BlueprintPure)
	FWFCTileDef2D GetTileDefByLocation(FIntPoint Location, int32& Index) const;

	UFUNCTION(BlueprintPure)
	FWFCTileDef2D GetTileDefByIndex(int32 Index) const;

	virtual void GetAllowedRotations(TArray<int32>& OutRotations) const override;
	virtual int32 GetNumTileDefs() const override { return TileDefs.Num(); }
	virtual FGameplayTag GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const override;
	virtual int32 GetTileDefInDirection(int32 TileDefIndex, FWFCGridDirection Direction) const override;
	virtual TArray<FActorInfoEntry> GetTileDefActorsInfo(int32 TileDefIndex) const override;
	virtual TArray<FStaticMeshInfoEntry> GetTileDefMeshesInfo(int32 TileDefIndex) const override;
	virtual TSubclassOf<UWFCUnit> GetTileDefUnit(int32 TileDefIndex) const override;
	virtual bool IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) override;
#endif
};
