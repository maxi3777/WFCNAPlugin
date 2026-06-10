// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"


class UWFCGenerator;
class FAsyncCellSelector;
class FAsyncModel;
class FAsyncGrid;
class FAsyncConstraint;


class FAsyncGeneratorContext
	: public TSharedFromThis<FAsyncGeneratorContext>
{
public:

	void Cancel()
	{
		bCancelled.Store(true);
	}

	bool IsValid() const
	{
		return !bCancelled.Load();
	}

private:

	TAtomic<bool> bCancelled{false};
};


/**
 * 
 */
class WFCNA_API FAsyncGenerator
{
public:
	FAsyncGenerator(TArray<FWFCCell> InCells, int32 InNumTiles, int32 InNumCells);
	virtual ~FAsyncGenerator();

	enum class EAsyncGeneratorState : uint8
	{
		None,
		/** A tile or tiles has not yet been selected. */
		InProgress,
		/** One or all tiles in question have been selected successfully. */
		Finished,
		/** A contradiction occured preventing the selection of a tile. */
		Error,
	};

	enum class EAsyncGeneratorStepPhase : uint8
	{
		None,
		Constraints,
		Selection,
	};
	
	EAsyncGeneratorState State;

	EAsyncGeneratorStepPhase CurrentStepPhase;

	void SetState(EAsyncGeneratorState NewState);

	FORCEINLINE int32 GetNumCells() const { return NumCells; }

	//保存初始状态（经过FixTile和Boundary）并使用其重置
	void Reset();
	
	/** [ForAsync] Run the generator until it is either finished, or an error occurs. */
	void Run(int32 StepLimit, TWeakObjectPtr<UWFCGenerator> WeakGenerator, TSharedRef<FAsyncGeneratorContext>& Context);

	/** [ForSync] Run the deterministic constraints for startup only, abort before any tile selection. */
	void RunStartup(int32 StepLimit);

	/** Continue the generator forward by selecting the next tile. */
	void Next(bool bNoSelection = false);

	/**
	 * Ban a tile from being a candidate for a cell.
	 * @return True if a contradiction was created.
	 */
	bool Ban(int32 CellIndex, int32 TileId);

	/**
	 * Ban multiple tiles from being candidates for a cell.
	 * @return True if a contradiction was created.
	 */
	bool BanMultiple(int32 CellIndex, TArray<int32> TileIds);

	/**
	 * Select a tile to use for a cell.
	 * This is equivalent to banning all other tile candidates.
	 */
	void Select(int32 CellIndex, int32 TileId);

	FORCEINLINE bool IsValidCellIndex(FWFCCellIndex Index) const { return Cells.IsValidIndex(Index); }

	FORCEINLINE bool IsValidTileId(FWFCTileId TileId) const { return TileId >= 0 && TileId < NumTiles; }

	/** Return cell data by index */
	FWFCCell& GetCell(FWFCCellIndex CellIndex);

	/** Return cell data by index */
	const FWFCCell& GetCell(FWFCCellIndex CellIndex) const;

	TSharedPtr<FAsyncModel> GetModel() const { return Model; }
	
	TSharedPtr<FAsyncModel> Model;
	
	/** The grid being used */
	TSharedPtr<FAsyncGrid> Grid;
	
	/** The constraint instances to apply, in order of priority. */
	TArray<TUniquePtr<FAsyncConstraint>> Constraints;

	//改为单一选择器
	/** The cell selector instances to use, in order of priority. */
	TUniquePtr<FAsyncCellSelector> CellSelector;
	
	/** The cached total number of available tiles. */
	int32 NumTiles;

	/** Array of all cells in the grid by cell index. */
	TArray<FWFCCell> Cells;

	/** The cached total number of cells in the grid */
	int32 NumCells;

	FRandomStream ThreadRandomStream;

	//---------Cache Initial State-----------

	TArray<FWFCCell> CellsCache;//这是经过FixTile和Boundary的

	//--------------------------------------
	
	bool AreAllCellsSelected() const;

	/** Called when a tile candidate has been banned from a cell. */
	virtual void OnCellCandidateBanned(FWFCCellIndex CellIndex, FWFCTileId BannedTileId);

	/** Called when tile candidates have been banned from a cell. */
	virtual void OnCellCandidatesBanned(FWFCCellIndex CellIndex, const TArray<FWFCTileId>& BannedTileIds);

	/** Called when the candidates for a cell have changed. */
	virtual void OnCellChanged(FWFCCellIndex CellIndex);

	/** Return the next cell that should be fully collapsed. */
	virtual FWFCCellIndex SelectNextCellIndex();

	/** Return the tile to select for a cell being collapsed. */
	virtual FWFCTileId SelectNextTileForCell(FWFCCellIndex CellIndex);

	
};
