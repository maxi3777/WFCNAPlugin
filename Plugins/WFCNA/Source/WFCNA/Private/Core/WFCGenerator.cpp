// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCGenerator.h"

#include "WFCModule.h"
#include "Core/AsyncGenerator.h"
#include "Core/WFCCellSelector.h"
#include "Core/WFCConstraint.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Stats/StatsMisc.h"
#include "Core/AsyncCellSelector.h"
#include "Core/AsyncConstraint.h"


// UWFCGenerator
// -------------

UWFCGenerator::UWFCGenerator()
	: State(EWFCGeneratorState::None),
	  NumTiles(0),
	  NumCells(0),
	  bIsInitialized(false)
{
}

void UWFCGenerator::SetState(EWFCGeneratorState NewState)
{
	if (State != NewState)
	{
		State = NewState;
		OnStateChanged.Broadcast(State);
	}
}

UWFCConstraint* UWFCGenerator::GetConstraint(TSubclassOf<UWFCConstraint> ConstraintClass) const
{
	for (UWFCConstraint* Constraint : Constraints)
	{
		if (Constraint->IsA(ConstraintClass))
		{
			return Constraint;
		}
	}
	return nullptr;
}

UWFCCellSelector* UWFCGenerator::GetCellSelector(TSubclassOf<UWFCCellSelector> SelectorClass) const
{
	if (CellSelector->IsA(SelectorClass))
	{
		return CellSelector;
	}
	return nullptr;
}

void UWFCGenerator::Configure(FWFCGeneratorConfig InConfig)
{
	Config = InConfig;
}

void UWFCGenerator::Initialize(bool bFull)
{
	if (bIsInitialized)
	{
		return;
	}

	// TODO: cache in WFCAsset snapshot, and then put this behind bFull
	Config.Model->GenerateTiles();
	NumTiles = Config.Model->GetNumTiles();

	InitializeGrid(Config.GridConfig.Get());
	InitializeCells();

	CreateConstraints();
	CreateCellSelector();

	if (bFull)
	{
		InitializeConstraints();
	}
	InitializeCellSelector();

	SetState(EWFCGeneratorState::InProgress);

	bIsInitialized = true;
}

void UWFCGenerator::InitializeGrid(const UWFCGridConfig* GridConfig)
{
	check(GridConfig != nullptr);
	Grid = UWFCGrid::NewGrid(this, GridConfig);
	check(Grid != nullptr);
}

void UWFCGenerator::CreateConstraints()
{
	Constraints.Reset();
	for (const TSubclassOf<UWFCConstraint>& ConstraintClass : Config.ConstraintClasses)
	{
		if (!ConstraintClass)
		{
			continue;
		}

		// create the new constraint object
		UWFCConstraint* Constraint = NewObject<UWFCConstraint>(this, ConstraintClass);
		check(Constraint != nullptr);
		Constraints.Add(Constraint);
	}
}

void UWFCGenerator::InitializeConstraints()
{
	for (UWFCConstraint* Constraint : Constraints)
	{
		Constraint->Initialize(this);
	}
}

void UWFCGenerator::CreateCellSelector()
{
	if (!Config.CellSelectorClass)
	{
		return;
	}

	UWFCCellSelector* NewSelector = NewObject<UWFCCellSelector>(this, Config.CellSelectorClass);
	check(NewSelector != nullptr);
	CellSelector = NewSelector;
}

void UWFCGenerator::InitializeCellSelector()
{
	CellSelector->Initialize();
}

void UWFCGenerator::InitializeCells()
{
	// generate array of all tile ids for default cell candidate list
	TArray<FWFCTileId> AllTileCandidates;
	AllTileCandidates.SetNum(GetNumTiles());
	for (int32 Idx = 0; Idx < AllTileCandidates.Num(); ++Idx)
	{
		// tile id is the same as the tile index
		AllTileCandidates[Idx] = Idx;
	}

	// fill the cells array
	NumCells = Grid->GetNumCells();
	Cells.SetNum(NumCells);
	for (FWFCCellIndex Idx = 0; Idx < NumCells; ++Idx)
	{
		Cells[Idx].TileCandidates = AllTileCandidates;
	}

	CacheCells = Cells;
}



void UWFCGenerator::ResetForRunAgain()
{
	Cells = CacheCells;
	SetState(EWFCGeneratorState::None);
}

TUniquePtr<FAsyncGenerator> UWFCGenerator::CreateAsyncGenerator(TArray<FWFCCell> InCells, int32 InNumTiles, int32 InNumCells)
{
	return MakeUnique<FAsyncGenerator>(InCells, InNumTiles, InNumCells);
}

void UWFCGenerator::StartCalculatingAsync(int32 MaxRetry,int32 StepLimit)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogWFC, Error, TEXT("Initialize must be called before Create a AsyncGenerator"));
		return;
	}
	
	TUniquePtr<FAsyncGenerator> AsyncGenerator = CreateAsyncGenerator(Cells, NumTiles, NumCells);

	AsyncGenerator->Model = Config.Model->CreateAsyncModel();
	AsyncGenerator->Grid = Grid->CreateAsyncGrid();
	AsyncGenerator->CellSelector = CellSelector->CreateAsyncCellSelector(AsyncGenerator.Get());
	for (UWFCConstraint* Constraint : Constraints)
	{
		AsyncGenerator->Constraints.Add(Constraint->CreateAsyncConstraint(AsyncGenerator.Get(), AsyncGenerator->Grid, AsyncGenerator->Model));
	}

	TWeakObjectPtr<UWFCGenerator> WeakThis(this);

	if (ActiveContext)
	{
		ActiveContext->Cancel();
	}
	ActiveContext = MakeShared<FAsyncGeneratorContext>();
	TSharedRef<FAsyncGeneratorContext> Context = ActiveContext.ToSharedRef();

	UE::Tasks::Launch(UE_SOURCE_LOCATION,[WeakThis, Context, AsyncGenerator = MoveTemp(AsyncGenerator), MaxRetry, StepLimit]() mutable
	{
		while (MaxRetry--)
		{
			if (!Context->IsValid()){return;}
			AsyncGenerator->Reset();
			AsyncGenerator->Run(StepLimit, WeakThis, Context);
			if (AsyncGenerator->State != FAsyncGenerator::EAsyncGeneratorState::Finished)
			{
				continue;
			}
			TArray<FWFCCell> ResultCells = AsyncGenerator->Cells;
			AsyncTask(ENamedThreads::GameThread,[WeakThis, Context, ResultCells = MoveTemp(ResultCells)]() mutable
			{
				if (!Context->IsValid()){return;}
				if (!WeakThis.IsValid())
				{
					UE_LOG(LogWFC, Error, TEXT("UWFCGenerator no exist, can not return AsyncValue"));
					return;
				}
				WeakThis->Cells = MoveTemp(ResultCells);
				WeakThis->SetState(EWFCGeneratorState::Finished);
			});
			return;
		}
		AsyncTask(ENamedThreads::GameThread,[WeakThis, Context]() mutable
		{
			if (!Context->IsValid()){return;}
			UE_LOG(LogWFC, Error, TEXT("Over MaxRetry, generate fail"));
			if (!WeakThis.IsValid())
			{
				UE_LOG(LogWFC, Error, TEXT("UWFCGenerator no exist, can not return AsyncState"));
				return;
			}
			WeakThis->SetState(EWFCGeneratorState::Error);
		});
	});
}

UWFCGeneratorSnapshot* UWFCGenerator::SyncStartupAndCreateSnapshot(UObject* Outer, int32 StepLimit)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogWFC, Error, TEXT("Initialize must be called before Create a AsyncGenerator"));
		return nullptr;
	}

	TUniquePtr<FAsyncGenerator> AsyncGenerator = CreateAsyncGenerator(Cells, NumTiles, NumCells);

	AsyncGenerator->Model = Config.Model->CreateAsyncModel();
	AsyncGenerator->Grid = Grid->CreateAsyncGrid();
	AsyncGenerator->CellSelector = CellSelector->CreateAsyncCellSelector(AsyncGenerator.Get());
	TArray<FAsyncConstraint*> AsyncConstraints;
	for (UWFCConstraint* Constraint : Constraints)
	{
		TUniquePtr<FAsyncConstraint> AsyncConstraint = Constraint->CreateAsyncConstraint(AsyncGenerator.Get(), AsyncGenerator->Grid, AsyncGenerator->Model);
		Constraint->AsyncConstraintForSnapshot = AsyncConstraint.Get();
		AsyncConstraints.Add(AsyncConstraint.Get());
		AsyncGenerator->Constraints.Add(MoveTemp(AsyncConstraint));
	}

	AsyncGenerator->RunStartup(StepLimit);

	if (AsyncGenerator->State == FAsyncGenerator::EAsyncGeneratorState::Error)
	{
		SetState(EWFCGeneratorState::Error);
		return nullptr;
	}

	return CreateSnapshot(Outer, AsyncGenerator.Get());
}

FWFCCell& UWFCGenerator::GetCell(FWFCCellIndex CellIndex)
{
	return Cells[CellIndex];
}

const FWFCCell& UWFCGenerator::GetCell(FWFCCellIndex CellIndex) const
{
	return Cells[CellIndex];
}

UWFCGeneratorSnapshot* UWFCGenerator::CreateSnapshot(UObject* Outer, FAsyncGenerator* AsyncGenerator) const
{
	UWFCGeneratorSnapshot* Snapshot = NewObject<UWFCGeneratorSnapshot>(Outer);
	Snapshot->Cells = AsyncGenerator->Cells;

	for (const UWFCConstraint* Constraint : Constraints)
	{
		if (UWFCConstraintSnapshot* ConstraintSnapshot = Constraint->CreateSnapshot(Snapshot))
		{
			Snapshot->ConstraintSnapshots.Add(Constraint->GetClass(), ConstraintSnapshot);
		}
	}

	return Snapshot;
}

void UWFCGenerator::ApplySnapshot(const UWFCGeneratorSnapshot* Snapshot)
{
	if (!Snapshot)
	{
		return;
	}

	if (Snapshot->Cells.Num() == !Cells.Num())
	{
		UE_LOG(LogWFC, Error, TEXT("Snapshot does not match cell count: %s"), *Snapshot->GetFullName(Snapshot->GetOuter()));
		return;
	}

	Cells = Snapshot->Cells;
	CacheCells = Snapshot->Cells;

	for (UWFCConstraint* Constraint : Constraints)
	{
		if (const UWFCConstraintSnapshot* ConstraintSnapshot = Snapshot->ConstraintSnapshots.FindRef(Constraint->GetClass()))
		{
			Constraint->ApplySnapshot(ConstraintSnapshot);
			UE_LOG(LogWFC, Verbose, TEXT("Applied constraint snapshot: %s"), *Constraint->GetName());
		}
	}

	UE_LOG(LogWFC, Log, TEXT("Applied snapshot: %s"), *Snapshot->GetFullName(Snapshot->GetOuter()));
}


FString UWFCGenerator::GetTileDebugString(int32 TileId) const
{
	if (Config.Model.IsValid())
	{
		if (const FWFCModelTile* ModelTile = Config.Model->GetTile(TileId))
		{
			return ModelTile->ToString();
		}
	}
	return FString();
}

void UWFCGenerator::StopAsyncGenerating()
{
	ActiveContext->Cancel();
}





