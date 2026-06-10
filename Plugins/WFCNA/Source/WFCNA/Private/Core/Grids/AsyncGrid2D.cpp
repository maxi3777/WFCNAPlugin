// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Grids/AsyncGrid2D.h"


FAsyncGrid2D::FAsyncGrid2D(FIntPoint InDimensions) : Dimensions(InDimensions)
{
}

FAsyncGrid2D::~FAsyncGrid2D()
{
}

int32 FAsyncGrid2D::GetNumCells() const
{
	return Dimensions.X * Dimensions.Y;
}

int32 FAsyncGrid2D::GetOppositeDirection(FWFCGridDirection Direction) const
{
	// {0, 1, 2, 3} represents {+X, +Y, -X, -Y}
	switch (Direction)
	{
	case 0: return 2;
	case 2: return 0;
	case 1: return 3;
	case 3: return 1;
	default: return INDEX_NONE;
	}
}

FWFCCellIndex FAsyncGrid2D::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
{
	if (!IsValidCellIndex(CellIndex) || !IsValidDirection(Direction))
	{
		// invalid cell or direction
		return INDEX_NONE;
	}

	const FIntPoint GridLocation = GetLocationForCellIndex(CellIndex);
	const FIntPoint MovedGridLocation = GridLocation + GetDirectionVectorStatic(Direction);
	return GetCellIndexForLocation(MovedGridLocation);
}

int32 FAsyncGrid2D::GetCellIndexForLocation(FIntPoint GridLocation) const
{
	if (GridLocation.X < 0 || GridLocation.X >= Dimensions.X ||
		GridLocation.Y < 0 || GridLocation.Y >= Dimensions.Y)
	{
		return INDEX_NONE;
	}
	return GridLocation.X + (GridLocation.Y * Dimensions.X);
}

FIntPoint FAsyncGrid2D::GetLocationForCellIndex(int32 CellIndex) const
{
	const int32 X = CellIndex % Dimensions.X;
	const int32 Y = (CellIndex - X) / Dimensions.X;
	return FIntPoint(X, Y);
}

FIntPoint FAsyncGrid2D::GetDirectionVectorStatic(int32 Direction)
{
	switch (Direction)
	{
	case 0:
		return FIntPoint(1, 0);
	case 1:
		return FIntPoint(0, 1);
	case 2:
		return FIntPoint(-1, 0);
	case 3:
		return FIntPoint(0, -1);
	default:
		return FIntPoint();
	}
}