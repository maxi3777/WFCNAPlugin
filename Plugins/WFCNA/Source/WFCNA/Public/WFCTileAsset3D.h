// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCTileAsset.h"
#include "WFCTileAsset2D.h"
#include "WFCTileDefType.h"
#include "GameFramework/Actor.h"
#include "WFCTileAsset3D.generated.h"


UENUM(BlueprintType)
enum class EWFCCubeRotationType : uint8
{
	One,   // 不旋转 (0°)
	Two,   // 两向旋转 (0°, 90°)
	Four   // 四向旋转 (0°, 90°, 180°, 270°)
};

/** The edge types for a 3D tile */
UENUM(BlueprintType)
enum class EWFCTile3DEdge : uint8
{
	XPos UMETA(DisplayName = "+X"),
	YPos UMETA(DisplayName = "+Y"),
	XNeg UMETA(DisplayName = "-X"),
	YNeg UMETA(DisplayName = "-Y"),
	ZPos UMETA(DisplayName = "+Z"),
	ZNeg UMETA(DisplayName = "-Z"),
	MAX UMETA(Hidden),
};


/**
 * Definition of a 3D tile unit.
 */
USTRUCT(BlueprintType)
struct FWFCTileDef3D
{
	GENERATED_BODY()

	FWFCTileDef3D()
		: Location(FIntVector::ZeroValue)
	{
		EdgeTypes = {
			{EWFCTile3DEdge::XPos, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::YPos, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::XNeg, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::YNeg, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::ZPos, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::ZNeg, FGameplayTag::EmptyTag},
		};
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FActorInfoEntry> ActorsInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStaticMeshInfoEntry> StaticMeshesInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UWFCUnit> WFCUnit;

	/** The types for all edges of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"))
	TMap<EWFCTile3DEdge, FGameplayTag> EdgeTypes;
};


/**
 * A 3D tile with a socket type on each side for use with adjacency constraints.
 * References an actor to be spawned for each tile.
 */
UCLASS()
class WFCNA_API UWFCTileAsset3D : public UWFCTileAsset
{
	GENERATED_BODY()

public:
	UWFCTileAsset3D();

	/**
	 * The dimensions of this tile. Tile assets can contain multiple actual tiles
	 * which will have fixed adjacency rules setup for them.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Dimensions;

	/** How this piece be rotating? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWFCCubeRotationType RotationType;

	/** The individual tiles that make up this group of tiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "Location"))
	TArray<FWFCTileDef3D> TileDefs;

	/** Return a tile def by location, as well as its index */
	UFUNCTION(BlueprintPure)
	FWFCTileDef3D GetTileDefByLocation(FIntVector Location, int32& Index) const;

	UFUNCTION(BlueprintPure)
	FWFCTileDef3D GetTileDefByIndex(int32 Index) const;

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