// Copyright maxi3777. All Rights Reserved.


#include "Core/AsyncModel.h"


FAsyncModel::FAsyncModel(int32 InNumTiles, TArray<float> InTileWeights) : NumTiles(InNumTiles), TileWeights(InTileWeights)
{
}

FAsyncModel::~FAsyncModel()
{
}
