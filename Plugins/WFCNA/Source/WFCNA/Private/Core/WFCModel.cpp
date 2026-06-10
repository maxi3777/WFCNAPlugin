// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026


#include "Core/WFCModel.h"

#include "Core/AsyncModel.h"


void UWFCModel::Initialize(const UObject* TileData)
{
	TileDataRef = TileData;
}

void UWFCModel::GenerateTiles()
{
}

FWFCTileId UWFCModel::AddTile(TSharedPtr<FWFCModelTile> Tile)
{
	check(Tile.IsValid());
	check(Tiles.Num() == TileWeights.Num());

	Tile->Id = Tiles.Num();
	Tiles.Add(Tile);
	TileWeights.Add(Tile->Weight);

	return Tile->Id;
}

FWFCModelTile* UWFCModel::GetTile(FWFCTileId TileId) const
{
	return Tiles.IsValidIndex(TileId) ? Tiles[TileId].Get() : nullptr;
}

FString UWFCModel::GetTileDebugString(FWFCTileId TileId) const
{
	return FString::Printf(TEXT("Tile %d"), TileId);
}

TSharedPtr<FAsyncModel> UWFCModel::CreateAsyncModel()
{
	return MakeShared<FAsyncModel>(Tiles.Num(), TileWeights);
}

