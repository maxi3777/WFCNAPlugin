// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AsyncModel.h"


FAsyncModel::FAsyncModel(int32 InNumTiles, TArray<float> InTileWeights) : NumTiles(InNumTiles), TileWeights(InTileWeights)
{
}

FAsyncModel::~FAsyncModel()
{
}
