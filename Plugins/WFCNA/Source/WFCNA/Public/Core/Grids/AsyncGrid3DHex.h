// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncGrid.h"
#include "Core/WFCTypes.h"

/**
 * 
 */
class WFCNA_API FAsyncGrid3DHex : public FAsyncGrid
{
public:
	explicit FAsyncGrid3DHex(FIntVector InDimensions);
	virtual ~FAsyncGrid3DHex() override;

	FIntVector Dimensions;

	virtual int32 GetNumCells() const override;
	FORCEINLINE virtual int32 GetNumDirections() const override { return 8; } // 6 Horizontal + 2 Vertical
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;

	virtual FIntVector GetDirectionVector(int32 Direction) const override; // 返回的是 Offset 坐标系的 Delta 吗？不，这里返回 Cube 坐标更安全用于逻辑

	// --- Hex Specific Helpers ---
	
	int32 GetCellIndexForLocation(FIntVector GridLocation) const;
	FIntVector GetLocationForCellIndex(int32 CellIndex) const;

	/** Offset (Col, Row) 转 Cube (q, r, s) */
	static FIntVector OffsetToCube(const FIntVector& Offset);

	/** Cube (q, r, s) 转 Offset (Col, Row) */
	static FIntVector CubeToOffset(const FIntVector& Cube);

	/** 旋转一个 Cube 坐标 (CW) */
	static FIntVector RotateCube(const FIntVector& Cube, int32 RotationSteps);

	/** 获取某个方向的 Cube 坐标增量 */
	static FIntVector GetDirectionVectorCube(int32 Direction);
};
