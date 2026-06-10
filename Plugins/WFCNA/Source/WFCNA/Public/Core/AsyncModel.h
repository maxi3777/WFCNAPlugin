// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCTypes.h"

/**
 * 
 */
class WFCNA_API FAsyncModel
{
public:
	explicit FAsyncModel(int32 InNumTiles, TArray<float> InTileWeights);
	~FAsyncModel();

	/** Return the weight of a tile. */
	FORCEINLINE float GetTileWeightUnchecked(FWFCTileId TileId) const { return TileWeights[TileId]; }

	int32 GetNumTiles() const { return NumTiles; }

protected:
	/** All generated tiles. Array index is the same as the tile id. */
	int32 NumTiles;
	
	/** All tile weights, by tile id. */
	TArray<float> TileWeights;
};
