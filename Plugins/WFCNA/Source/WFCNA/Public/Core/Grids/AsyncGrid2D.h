// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncGrid.h"
#include "Core/WFCTypes.h"

/**
 * 
 */
class WFCNA_API FAsyncGrid2D : public FAsyncGrid
{
public:
	explicit FAsyncGrid2D(FIntPoint InDimensions);
	virtual ~FAsyncGrid2D() override;

	FIntPoint Dimensions;

	virtual int32 GetNumCells() const override;
	FORCEINLINE virtual int32 GetNumDirections() const override { return 4; }
	
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;

	/** Return the cell index for a grid location */
	int32 GetCellIndexForLocation(FIntPoint GridLocation) const;

	/** Return the grid location for a cell */
	FIntPoint GetLocationForCellIndex(int32 CellIndex) const;

	static FIntPoint GetDirectionVectorStatic(int32 Direction);
};
