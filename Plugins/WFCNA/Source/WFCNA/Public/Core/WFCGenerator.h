// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "WFCGenerator.generated.h"

class FAsyncGeneratorContext;
class FAsyncConstraint;
class FAsyncGenerator;
class UWFCConstraintSnapshot;
class UWFCCellSelector;
class UWFCModel;
class UWFCGrid;
class UWFCGridConfig;
class UWFCConstraint;


/** Stores information about a generator that can be used to restore state. */
UCLASS(DefaultToInstanced, EditInlineNew)
class WFCNA_API UWFCGeneratorSnapshot : public UObject
{
	GENERATED_BODY()

public:
	TArray<FWFCCell> Cells;

	/** Snapshots for each of the constraints, by class. */
	UPROPERTY(VisibleAnywhere)
	TMap<TSubclassOf<UWFCConstraint>, TObjectPtr<UWFCConstraintSnapshot>> ConstraintSnapshots;
};

/**
 * Required objects and settings for initializing a WFCGenerator.
 */
USTRUCT(BlueprintType)
struct FWFCGeneratorConfig
{
	GENERATED_BODY()

	FWFCGeneratorConfig()
	{
	}

	UPROPERTY()
	TWeakObjectPtr<UWFCModel> Model;

	UPROPERTY()
	TWeakObjectPtr<const UWFCGridConfig> GridConfig;

	UPROPERTY()
	TArray<TSubclassOf<UWFCConstraint>> ConstraintClasses;

	UPROPERTY()
	TSubclassOf<UWFCCellSelector> CellSelectorClass;
};



/**
 * Handles running the actual processes for selecting, banning, and propagating
 * changes for a WFC model, grid, and tile set.
 */
UCLASS(BlueprintType, Blueprintable)
class WFCNA_API UWFCGenerator : public UObject
{
	GENERATED_BODY()

public:
	UWFCGenerator();

	/** The configuration supplied to this generator on initialize. */
	UPROPERTY(BlueprintReadOnly)
	FWFCGeneratorConfig Config;

	/** The current state of the generator. */
	UPROPERTY(BlueprintReadOnly)
	EWFCGeneratorState State;

	void SetState(EWFCGeneratorState NewState);

	/** Return the total number of cells */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumCells() const { return NumCells; }

	/** Return the total number of unique tile types. */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumTiles() const { return NumTiles; }

	/** Return the model object, containing all expanded tiles. */
	UFUNCTION(BlueprintPure)
	const UWFCModel* GetModel() const { return Config.Model.Get(); }

	/** Return the grid config of the source asset. */
	UFUNCTION(BlueprintPure)
	const UWFCGridConfig* GetGridConfig() const { return Config.GridConfig.Get(); }

	template <class T>
	const T* GetModel() const
	{
		return Cast<T>(GetModel());
	}

	FORCEINLINE const UWFCGrid* GetGrid() const { return Grid; }

	template <class T>
	const T* GetGrid() const
	{
		return Cast<T>(GetGrid());
	}

	/** Return a constraint by class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType="ConstraintClass"))
	UWFCConstraint* GetConstraint(TSubclassOf<UWFCConstraint> ConstraintClass) const;

	/** Return a cell selector by class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType="SelectorClass"))
	UWFCCellSelector* GetCellSelector(TSubclassOf<UWFCCellSelector> SelectorClass) const;

	/** Set the config. Should be called before Initialize. */
	void Configure(FWFCGeneratorConfig InConfig);

	/**
	 * Initialize the generator.
	 * @param bFull If true, perform a full initialization of the model and constraints, otherwise those must be done manually.
	 */
	UFUNCTION(BlueprintCallable)
	void Initialize(bool bFull = true);

	/** Initialize all constraints. */
	UFUNCTION(BlueprintCallable)
	void InitializeConstraints();

	UFUNCTION(BlueprintPure)
	bool IsInitialized() const { return bIsInitialized; }

	/** Reset all cells and constraints to their initialized state. */
	UFUNCTION(BlueprintCallable)
	void ResetForRunAgain();
	
	static TUniquePtr<FAsyncGenerator> CreateAsyncGenerator(TArray<FWFCCell> InCells, int32 InNumTiles, int32 InNumCells);

	//在GT中用SetState将State改为Finish或Error,返回Cells
	void StartCalculatingAsync(int32 MaxRetry = 50, int32 StepLimit = 100000);

	//为了创建Snapshot的GT同步运行
	UWFCGeneratorSnapshot* SyncStartupAndCreateSnapshot(UObject* Outer, int32 StepLimit = 100000);

	FORCEINLINE bool IsValidCellIndex(FWFCCellIndex Index) const { return Cells.IsValidIndex(Index); }

	FORCEINLINE bool IsValidTileId(FWFCTileId TileId) const { return TileId >= 0 && TileId < NumTiles; }

	/** Return cell data by index */
	FWFCCell& GetCell(FWFCCellIndex CellIndex);
	const FWFCCell& GetCell(FWFCCellIndex CellIndex) const;

	/** Create an return a snapshot of this generator. */
	UWFCGeneratorSnapshot* CreateSnapshot(UObject* Outer, FAsyncGenerator* AsyncGenerator) const;

	/** Update the state of this generator to a previous snapshot. */
	void ApplySnapshot(const UWFCGeneratorSnapshot* Snapshot);

	FORCEINLINE TArray<TObjectPtr<UWFCConstraint>>& GetConstraints() { return Constraints; }

	FORCEINLINE const TArray<TObjectPtr<UWFCConstraint>>& GetConstraints() const { return Constraints; }

	FString GetTileDebugString(int32 TileId) const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FStateChangedDelegate, EWFCGeneratorState /* State */);

	/** Called when the state has changed */
	FStateChangedDelegate OnStateChanged;

	UFUNCTION(BlueprintCallable)
	void StopAsyncGenerating();

protected:

	/** 用于管理（取消）Tasks */
	TSharedPtr<FAsyncGeneratorContext> ActiveContext;
	
	/** The grid being used */
	UPROPERTY(Transient)
	TObjectPtr<UWFCGrid> Grid;

	/** The constraint instances to apply, in order of priority. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UWFCConstraint>> Constraints;

	/** The cell selector instances to use, in order of priority. */
	UPROPERTY(Transient)
	TObjectPtr<UWFCCellSelector> CellSelector;

	/** The cached total number of available tiles. */
	int32 NumTiles;

	/** Array of all cells in the grid by cell index. */
	TArray<FWFCCell> Cells;

	/** Cached the Cells for Reset. */
	TArray<FWFCCell> CacheCells;

	/** The cached total number of cells in the grid */
	int32 NumCells;

	bool bIsInitialized;

	/** Create and initialize the grid. */
	virtual void InitializeGrid(const UWFCGridConfig* GridConfig);

	virtual void CreateConstraints();

	virtual void CreateCellSelector();

	/** Create and initialize cell selector objects. */
	virtual void InitializeCellSelector();

	/** Populate the cells array with default values for every cell in the grid */
	virtual void InitializeCells();



public:
	template <class T>
	T* GetConstraint()
	{
		T* Result = nullptr;
		if (Constraints.FindItemByClass(&Result))
		{
			return Result;
		}
		return nullptr;
	}

	template <class T>
	const T* GetConstraint() const
	{
		const T* Result = nullptr;
		if (Constraints.FindItemByClass(&Result))
		{
			return Result;
		}
		return nullptr;
	}
};
