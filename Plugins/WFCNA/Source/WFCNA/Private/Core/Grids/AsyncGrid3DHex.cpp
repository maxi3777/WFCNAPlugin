// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Grids/AsyncGrid3DHex.h"


FAsyncGrid3DHex::FAsyncGrid3DHex(FIntVector InDimensions) : Dimensions(InDimensions)
{
}

FAsyncGrid3DHex::~FAsyncGrid3DHex()
{
}

int32 FAsyncGrid3DHex::GetNumCells() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

int32 FAsyncGrid3DHex::GetOppositeDirection(FWFCGridDirection Direction) const
{
	if (Direction < 6) return (Direction + 3) % 6; // 水平方向对侧
	if (Direction == 6) return 7; // Top -> Bottom
	if (Direction == 7) return 6; // Bottom -> Top
	return INDEX_NONE;
}

FWFCCellIndex FAsyncGrid3DHex::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
{
	if (!IsValidCellIndex(CellIndex) || !IsValidDirection(Direction)) return INDEX_NONE;

	FIntVector CurrentOffset = GetLocationForCellIndex(CellIndex);

	// 处理垂直方向 (简单 Z 轴加减)
	if (Direction == 6) // Top
	{
		FIntVector Next = CurrentOffset + FIntVector(0, 0, 1);
		return GetCellIndexForLocation(Next);
	}
	if (Direction == 7) // Bottom
	{
		FIntVector Next = CurrentOffset + FIntVector(0, 0, -1);
		return GetCellIndexForLocation(Next);
	}

	// 处理水平方向 (需要转 Cube -> 加向量 -> 转回 Offset)
	FIntVector Cube = OffsetToCube(CurrentOffset);
	FIntVector DirVec = GetDirectionVectorCube(Direction);
	FIntVector NextCube = Cube + DirVec; // 注意：这里 FIntVector 只有 XYZ，Cube 的 s 分量存在 Z 里，加法是分量相加，逻辑成立
    
	FIntVector NextOffset2D = CubeToOffset(NextCube);
	FIntVector NextOffset = FIntVector(NextOffset2D.X, NextOffset2D.Y, CurrentOffset.Z);

	return GetCellIndexForLocation(NextOffset);
}

int32 FAsyncGrid3DHex::GetCellIndexForLocation(FIntVector GridLocation) const
{
    if (GridLocation.X < 0 || GridLocation.X >= Dimensions.X ||
        GridLocation.Y < 0 || GridLocation.Y >= Dimensions.Y ||
        GridLocation.Z < 0 || GridLocation.Z >= Dimensions.Z)
    {
        return INDEX_NONE;
    }
    return GridLocation.X + (GridLocation.Y * Dimensions.X) + (GridLocation.Z * Dimensions.X * Dimensions.Y);
}

FIntVector FAsyncGrid3DHex::GetLocationForCellIndex(int32 CellIndex) const
{
    const int32 DimXY = Dimensions.X * Dimensions.Y;
    const int32 Z = CellIndex / DimXY;
    const int32 Remainder = CellIndex % DimXY;
    const int32 Y = Remainder / Dimensions.X;
    const int32 X = Remainder % Dimensions.X;
    return FIntVector(X, Y, Z);
}

FIntVector FAsyncGrid3DHex::GetDirectionVector(int32 Direction) const
{
    // 仅用于接口兼容，不建议直接使用，因为 Hex 的 Offset 坐标增量依赖于行号的奇偶性
    // 这里返回 Zero 强迫调用者使用 GetCellIndexInDirection
    return FIntVector::ZeroValue;
}

FIntVector FAsyncGrid3DHex::OffsetToCube(const FIntVector& Offset)
{
	// Odd-r conversion
	// q = col - (row - (row&1)) / 2
	// r = row
	// s = -q - r
	int32 q = Offset.X - (Offset.Y - (Offset.Y & 1)) / 2;
	int32 r = Offset.Y;
	int32 s = -q - r;
	return FIntVector(q, r, s);
}

FIntVector FAsyncGrid3DHex::CubeToOffset(const FIntVector& Cube)
{
	// Odd-r conversion
	// col = q + (r - (r&1)) / 2
	// row = r
	int32 col = Cube.X + (Cube.Y - (Cube.Y & 1)) / 2;
	int32 row = Cube.Y;
	// Z 轴透传
	return FIntVector(col, row, 0); 
}

FIntVector FAsyncGrid3DHex::RotateCube(const FIntVector& Cube, int32 RotationSteps)
{
	// 在六边形 Cube 坐标中，顺时针旋转 60度 相当于坐标分量交换并取反
	// (q, r, s) -> (-r, -s, -q)
    
	int32 q = Cube.X;
	int32 r = Cube.Y;
	int32 s = Cube.Z;

	int32 Steps = RotationSteps % 6;
	for(int32 i=0; i<Steps; ++i)
	{
		int32 new_q = -r;
		int32 new_r = -s;
		int32 new_s = -q;
		q = new_q; r = new_r; s = new_s;
	}
	return FIntVector(q, r, s);
}

FIntVector FAsyncGrid3DHex::GetDirectionVectorCube(int32 Direction)
{
	// Cube: X=q, Y=r, Z=s (约束: q+r+s=0)
	// 对应 Odd-r Pointy Top 的方向定义
	switch (Direction)
	{
	case 0: return FIntVector(1, 0, -1);  // East
	case 1: return FIntVector(0, 1, -1);  // SouthEast
	case 2: return FIntVector(-1, 1, 0);  // SouthWest
	case 3: return FIntVector(-1, 0, 1);  // West
	case 4: return FIntVector(0, -1, 1);  // NorthWest
	case 5: return FIntVector(1, -1, 0);  // NorthEast
		// 垂直方向仅用于标识，不参与 Cube 平面运算，这里返回 Zero
	default: return FIntVector(0, 0, 0); 
	}
}