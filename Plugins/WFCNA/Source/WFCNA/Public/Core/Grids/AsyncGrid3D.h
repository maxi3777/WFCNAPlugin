// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncGrid.h"

/**
 * 
 */
class WFCNA_API FAsyncGrid3D : public FAsyncGrid
{
public:
	explicit FAsyncGrid3D(FIntVector InDimensions);
	virtual ~FAsyncGrid3D() override;

	FIntVector Dimensions;

	virtual int32 GetNumCells() const override;
	FORCEINLINE virtual int32 GetNumDirections() const override { return 6; }

	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;

	int32 GetCellIndexForLocation(FIntVector GridLocation) const;
	FIntVector GetLocationForCellIndex(int32 CellIndex) const;
	virtual FIntVector GetDirectionVector(int32 Direction) const override;

	static FIntVector GetDirectionVectorStatic(int32 Direction);
};
