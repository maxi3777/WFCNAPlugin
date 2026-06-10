// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026


#include "WFCTileAsset2D.h"

#include "Core/Grids/WFCGrid2D.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "WFCEditor"


UWFCTileAsset2D::UWFCTileAsset2D()
	: Dimensions(FIntPoint(1, 1)),
	  RotationType(EWFCSquareRotationType::Four)
{
}

FWFCTileDef2D UWFCTileAsset2D::GetTileDefByLocation(FIntPoint Location, int32& Index) const
{
	Index = INDEX_NONE;
	for (int32 Idx = 0; Idx < TileDefs.Num(); ++Idx)
	{
		if (TileDefs[Idx].Location == Location)
		{
			Index = Idx;
			return TileDefs[Idx];
		}
	}
	return FWFCTileDef2D();
}

FWFCTileDef2D UWFCTileAsset2D::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef2D();
}

void UWFCTileAsset2D::GetAllowedRotations(TArray<int32>& OutRotations) const
{
	OutRotations.Empty();
	switch (RotationType)
	{
	case EWFCSquareRotationType::One:
		OutRotations = {0};
		break;
	case EWFCSquareRotationType::Two:
		OutRotations = {0, 1}; // 0°, 90° (旋转步数0和1)
		break;
	case EWFCSquareRotationType::Four:
	default:
		OutRotations = {0, 1, 2, 3}; // 0°, 90°, 180°, 270°
		break;
	}
}

FGameplayTag UWFCTileAsset2D::GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].EdgeTypes[static_cast<EWFCTile2DEdge>(Direction)];
}

int32 UWFCTileAsset2D::GetTileDefInDirection(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	const FIntPoint TargetLocation = TileDefs[TileDefIndex].Location + UWFCGrid2D::GetDirectionVectorStatic(Direction);
	for (int32 Idx = 0; Idx < TileDefs.Num(); ++Idx)
	{
		if (TileDefs[Idx].Location == TargetLocation)
		{
			return Idx;
		}
	}
	return INDEX_NONE;
}

TArray<FActorInfoEntry> UWFCTileAsset2D::GetTileDefActorsInfo(int32 TileDefIndex) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].ActorsInfo;
}

TArray<FStaticMeshInfoEntry> UWFCTileAsset2D::GetTileDefMeshesInfo(int32 TileDefIndex) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].StaticMeshesInfo;
}

TSubclassOf<UWFCUnit> UWFCTileAsset2D::GetTileDefUnit(int32 TileDefIndex) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].WFCUnit;
}

bool UWFCTileAsset2D::IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	const FIntPoint DirectionVector = UWFCGrid2D::GetDirectionVectorStatic(Direction);
	const FIntPoint TileLocation = TileDefs[TileDefIndex].Location;
	const FIntPoint DeltaLocation = TileLocation + DirectionVector;
	return DeltaLocation.X >= 0 && DeltaLocation.X < Dimensions.X &&
		DeltaLocation.Y >= 0 && DeltaLocation.Y < Dimensions.Y;
}


#if WITH_EDITOR
EDataValidationResult UWFCTileAsset2D::IsDataValid(FDataValidationContext& Context)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (TileDefs.Num() != Dimensions.X * Dimensions.Y)
	{
		Context.AddError(LOCTEXT("IncorrectTileDefCount", "Tile def count does not match tile dimensions"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
