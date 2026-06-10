// Copyright maxi3777. All Rights Reserved.


#include "Core/Grids/AsyncGrid3D.h"


FAsyncGrid3D::FAsyncGrid3D(FIntVector InDimensions) : Dimensions(InDimensions)
{
}

FAsyncGrid3D::~FAsyncGrid3D()
{
}

int32 FAsyncGrid3D::GetNumCells() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

int32 FAsyncGrid3D::GetOppositeDirection(FWFCGridDirection Direction) const
{
	// {0, 1, 2, 3, 4, 5} represents {+X, +Y, -X, -Y, +Z, -Z}
	switch (Direction)
	{
	case 0:
		return 2;
	case 2:
		return 0;
	case 1:
		return 3;
	case 3:
		return 1;
	case 4:
		return 5;
	case 5:
		return 4;
	default:
		return INDEX_NONE;
	}
}

FWFCCellIndex FAsyncGrid3D::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
{
	if (!IsValidCellIndex(CellIndex) || !IsValidDirection(Direction))
	{
		// invalid cell or direction
		return INDEX_NONE;
	}

	const FIntVector GridLocation = GetLocationForCellIndex(CellIndex);
	const FIntVector MovedGridLocation = GridLocation + GetDirectionVector(Direction);
	return GetCellIndexForLocation(MovedGridLocation);
}

int32 FAsyncGrid3D::GetCellIndexForLocation(FIntVector GridLocation) const
{
	if (GridLocation.X < 0 || GridLocation.X >= Dimensions.X ||
		GridLocation.Y < 0 || GridLocation.Y >= Dimensions.Y ||
		GridLocation.Z < 0 || GridLocation.Z >= Dimensions.Z)
	{
		return INDEX_NONE;
	}
	return GridLocation.X + (GridLocation.Y * Dimensions.X) + (GridLocation.Z * Dimensions.X * Dimensions.Y);
}

FIntVector FAsyncGrid3D::GetLocationForCellIndex(int32 CellIndex) const
{
	// TODO: simplify
	const int32 DimXY = Dimensions.X * Dimensions.Y;
	const int32 Z = FMath::FloorToInt(static_cast<float>(CellIndex) / static_cast<float>(DimXY));
	const int32 Y = FMath::FloorToInt(static_cast<float>(CellIndex - Z * DimXY) / static_cast<float>(Dimensions.X));
	const int32 X = CellIndex % Dimensions.X;
	return FIntVector(X, Y, Z);
}

FIntVector FAsyncGrid3D::GetDirectionVector(int32 Direction) const
{
	return GetDirectionVectorStatic(Direction);
}

FIntVector FAsyncGrid3D::GetDirectionVectorStatic(int32 Direction)
{
	switch (Direction)
	{
	case 0:
		return FIntVector(1, 0, 0);
	case 1:
		return FIntVector(0, 1, 0);
	case 2:
		return FIntVector(-1, 0, 0);
	case 3:
		return FIntVector(0, -1, 0);
	case 4:
		return FIntVector(0, 0, 1);
	case 5:
		return FIntVector(0, 0, -1);
	default:
		return FIntVector();
	}
}