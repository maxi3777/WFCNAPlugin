// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"

class FAsyncModel;
class FAsyncGrid;
class FAsyncGenerator;
/**
 * 
 */
class WFCNA_API FAsyncConstraint
{
public:
	FAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel);
	virtual ~FAsyncConstraint();

	FAsyncGenerator* GetGenerator() const { return Generator; }

	/** Reset the constraint to its initialized state */
	virtual void Reset();

	/** Called when a cell's tile candidates have changed */
	virtual void NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection);

	/** Called when a tile candidate has been banned from a cell. */
	virtual void NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId);

	/**
	 * Update the constraint
	 * @return True if the constraint made any changes, false otherwise.
	 */
	virtual bool Next();

protected:

	FAsyncGenerator* Generator;

	/** Reference to the grid being used. */
	TSharedPtr<FAsyncGrid> Grid;

	/** Reference to the model being used. */
	TSharedPtr<FAsyncModel> Model;
};
