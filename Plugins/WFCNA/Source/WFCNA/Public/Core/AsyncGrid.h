// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCTypes.h"

/**
 * 
 */
class WFCNA_API FAsyncGrid
{
public:
	FAsyncGrid();
	virtual ~FAsyncGrid();

	virtual int32 GetNumCells() const { return 0; }
	/** Return true if a cell index is valid */
	FORCEINLINE bool IsValidCellIndex(FWFCCellIndex CellIndex) const
	{
		return CellIndex >= 0 && CellIndex < GetNumCells();
	}

	virtual int32 GetNumDirections() const { return 0; }
	/** Return true if a direction is valid for a cell */
	FORCEINLINE bool IsValidDirection(FWFCGridDirection Direction) const
	{
		return Direction >= 0 && Direction < GetNumDirections();
	}


	/** Return the direction that goes the opposite way of a direction. */
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const;

	/** Return the index of the cell that is one unit in a direction from another cell. */
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const;

	virtual FIntVector GetDirectionVector(int32 Direction) const;
};
