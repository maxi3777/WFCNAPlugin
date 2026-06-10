// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCTileAsset3D.h"

#include "Core/Grids/WFCGrid3D.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "WFCEditor"


UWFCTileAsset3D::UWFCTileAsset3D()
	: Dimensions(FIntVector(1, 1, 1)),
	  RotationType(EWFCCubeRotationType::Four)
{
}

FWFCTileDef3D UWFCTileAsset3D::GetTileDefByLocation(FIntVector Location, int32& Index) const
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
	return FWFCTileDef3D();
}

FWFCTileDef3D UWFCTileAsset3D::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef3D();
}

void UWFCTileAsset3D::GetAllowedRotations(TArray<int32>& OutRotations) const
{
	OutRotations.Empty();
	switch (RotationType)
	{
	case EWFCCubeRotationType::One:
		OutRotations = {0};
		break;
	case EWFCCubeRotationType::Two:
		OutRotations = {0, 1}; // 0°, 90°
		break;
	case EWFCCubeRotationType::Four:
	default:
		OutRotations = {0, 1, 2, 3}; // 0°, 90°, 180°, 270°
		break;
	}
}

FGameplayTag UWFCTileAsset3D::GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].EdgeTypes[static_cast<EWFCTile3DEdge>(Direction)];
}

int32 UWFCTileAsset3D::GetTileDefInDirection(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	const FIntVector TargetLocation = TileDefs[TileDefIndex].Location + UWFCGrid3D::GetDirectionVectorStatic(Direction);
	for (int32 Idx = 0; Idx < TileDefs.Num(); ++Idx)
	{
		if (TileDefs[Idx].Location == TargetLocation)
		{
			return Idx;
		}
	}
	return INDEX_NONE;
}

TArray<FActorInfoEntry> UWFCTileAsset3D::GetTileDefActorsInfo(int32 TileDefIndex) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].ActorsInfo;
}

TArray<FStaticMeshInfoEntry> UWFCTileAsset3D::GetTileDefMeshesInfo(int32 TileDefIndex) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].StaticMeshesInfo;
}

TSubclassOf<UWFCUnit> UWFCTileAsset3D::GetTileDefUnit(int32 TileDefIndex) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].WFCUnit;
}

bool UWFCTileAsset3D::IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	const FIntVector DirectionVector = UWFCGrid3D::GetDirectionVectorStatic(Direction);
	const FIntVector TileLocation = TileDefs[TileDefIndex].Location;
	const FIntVector DeltaLocation = TileLocation + DirectionVector;
	return DeltaLocation.X >= 0 && DeltaLocation.X < Dimensions.X &&
		DeltaLocation.Y >= 0 && DeltaLocation.Y < Dimensions.Y &&
		DeltaLocation.Z >= 0 && DeltaLocation.Z < Dimensions.Z;
}

#if WITH_EDITOR
EDataValidationResult UWFCTileAsset3D::IsDataValid(FDataValidationContext& Context)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (TileDefs.Num() != Dimensions.X * Dimensions.Y * Dimensions.Z)
	{
		Context.AddError(LOCTEXT("IncorrectTileDefCount", "Tile def count does not match tile dimensions"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
