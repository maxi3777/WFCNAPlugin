// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AsyncGenerator.h"

#include "Core/AsyncCellSelector.h"
#include "Core/AsyncConstraint.h"
#include "Core/AsyncModel.h"

//基于此建立Generator Snapshot
FAsyncGenerator::FAsyncGenerator(TArray<FWFCCell> InCells, int32 InNumTiles, int32 InNumCells)
	: State(EAsyncGeneratorState::None),
	  CurrentStepPhase(EAsyncGeneratorStepPhase::None),
	  NumTiles(InNumTiles),
	  Cells(InCells),
	  NumCells(InNumCells),
	  CellsCache(InCells)
{
}

FAsyncGenerator::~FAsyncGenerator()
{
}

void FAsyncGenerator::SetState(EAsyncGeneratorState NewState)
{
	if (State != NewState)
	{
		State = NewState;
	}
}

void FAsyncGenerator::Reset()
{
	Cells = CellsCache;
	
	for (TUniquePtr<FAsyncConstraint>& Constraint : Constraints)
	{
		Constraint->Reset();
	}
	
	CurrentStepPhase = EAsyncGeneratorStepPhase::None;
	SetState(EAsyncGeneratorState::None);
}

FWFCCell& FAsyncGenerator::GetCell(FWFCCellIndex CellIndex)
{
	return Cells[CellIndex];
}

const FWFCCell& FAsyncGenerator::GetCell(FWFCCellIndex CellIndex) const
{
	return Cells[CellIndex];
}

bool FAsyncGenerator::AreAllCellsSelected() const
{
	for (const FWFCCell& Cell : Cells)
	{
		if (!Cell.HasSelection())
		{
			return false;
		}
	}
	return true;
}

void FAsyncGenerator::Run(int32 StepLimit, TWeakObjectPtr<UWFCGenerator> WeakGenerator, TSharedRef<FAsyncGeneratorContext>& Context)
{
	for (int32 Step = 0; Step < StepLimit; ++Step)
	{
		if (!Context->IsValid() || !WeakGenerator.IsValid()){return;}
		Next();

		if (State != EAsyncGeneratorState::InProgress &&
			State != EAsyncGeneratorState::None)
		{
			break;
		}
	}
}

void FAsyncGenerator::RunStartup(int32 StepLimit)
{
	for (int32 Step = 0; Step < StepLimit; ++Step)
	{
		Next(true);

		if (CurrentStepPhase == EAsyncGeneratorStepPhase::Selection)
		{
			return;
		}

		if (State != EAsyncGeneratorState::InProgress &&
			State != EAsyncGeneratorState::None)
		{
			break;
		}
	}
}

void FAsyncGenerator::Next(bool bNoSelection)
{
	if (State == EAsyncGeneratorState::Finished)
	{
		return;
	}

	CurrentStepPhase = EAsyncGeneratorStepPhase::Constraints;

	// update all constraints, which may lead to cell selection
	for (TUniquePtr<FAsyncConstraint>& Constraint : Constraints)
	{
		if (Constraint->Next())
		{
			if (State == EAsyncGeneratorState::Finished || State == EAsyncGeneratorState::Error)
			{
				return;
			}

			SetState(EAsyncGeneratorState::InProgress);
		}
	}

	CurrentStepPhase = EAsyncGeneratorStepPhase::Selection;

	if (bNoSelection)
	{
		return;
	}

	// select a cell to observe
	const FWFCCellIndex CellIndex = SelectNextCellIndex();

	if (CellIndex == INDEX_NONE)
	{
		SetState(EAsyncGeneratorState::Error);
		return;
	}

	// select the tile for the cell
	const FWFCTileId TileId = SelectNextTileForCell(CellIndex);

	if (TileId == INDEX_NONE)
	{
		SetState(EAsyncGeneratorState::Error);
		return;
	}

	Select(CellIndex, TileId);

	if (State == EAsyncGeneratorState::Finished || State == EAsyncGeneratorState::Error)
	{
		return;
	}

	SetState(EAsyncGeneratorState::InProgress);
}

bool FAsyncGenerator::Ban(int32 CellIndex, int32 TileId)
{
	if (IsValidCellIndex(CellIndex))
	{
		FWFCCell& Cell = GetCell(CellIndex);
		if (Cell.RemoveCandidate(TileId))
		{
			OnCellCandidateBanned(CellIndex, TileId);
		}

		return Cell.HasNoCandidates();
	}
	return false;
}

bool FAsyncGenerator::BanMultiple(int32 CellIndex, TArray<int32> TileIds)
{
	bool bIsContradiction = false;
	if (IsValidCellIndex(CellIndex) && TileIds.Num() > 0)
	{
		TArray<FWFCTileId> BannedTileIds;
		BannedTileIds.Reserve(TileIds.Num());

		FWFCCell& Cell = GetCell(CellIndex);
		for (const int32& TileId : TileIds)
		{
			if (Cell.RemoveCandidate(TileId))
			{
				BannedTileIds.Add(TileId);
			}

			bIsContradiction |= Cell.HasNoCandidates();
		}

		if (!BannedTileIds.IsEmpty())
		{
			OnCellCandidatesBanned(CellIndex, BannedTileIds);
		}
	}
	return bIsContradiction;
}

void FAsyncGenerator::Select(int32 CellIndex, int32 TileId)
{
	if (IsValidCellIndex(CellIndex))
	{
		FWFCCell& Cell = GetCell(CellIndex);
		TArray<FWFCTileId> IdsToBan;
		for (const FWFCTileId& Id : Cell.TileCandidates)
		{
			if (Id != TileId)
			{
				IdsToBan.Add(Id);
			}
		}
		if (IdsToBan.Num() > 0)
		{
			BanMultiple(CellIndex, IdsToBan);
		}
	}
}

void FAsyncGenerator::OnCellCandidateBanned(FWFCCellIndex CellIndex, FWFCTileId BannedTileId)
{
	for (TUniquePtr<FAsyncConstraint>& Constraint : Constraints)
	{
		Constraint->NotifyCellBan(CellIndex, BannedTileId);
	}

	OnCellChanged(CellIndex);
}

void FAsyncGenerator::OnCellCandidatesBanned(FWFCCellIndex CellIndex, const TArray<FWFCTileId>& BannedTileIds)
{
	for (TUniquePtr<FAsyncConstraint>& Constraint : Constraints)
	{
		for (const FWFCTileId& BannedTileId : BannedTileIds)
		{
			Constraint->NotifyCellBan(CellIndex, BannedTileId);
		}
	}

	OnCellChanged(CellIndex);
}

void FAsyncGenerator::OnCellChanged(FWFCCellIndex CellIndex)
{
	FWFCCell& Cell = GetCell(CellIndex);
	const bool bHasSelection = Cell.HasSelection();
	if (Cell.HasNoCandidates())
	{
		// contradiction
		SetState(EAsyncGeneratorState::Error);
	}

	for (TUniquePtr<FAsyncConstraint>& Constraint : Constraints)
	{
		Constraint->NotifyCellChanged(CellIndex, bHasSelection);
	}

	if (AreAllCellsSelected())
	{
		SetState(EAsyncGeneratorState::Finished);
	}
}

FWFCCellIndex FAsyncGenerator::SelectNextCellIndex()
{
	if (const FWFCCellIndex CellIndex = CellSelector->SelectNextCell(); CellIndex != INDEX_NONE)
	{
		return CellIndex;
	}
	return INDEX_NONE;
}

FWFCTileId FAsyncGenerator::SelectNextTileForCell(FWFCCellIndex CellIndex)
{
	const FWFCCell& Cell = GetCell(CellIndex);

	if (Cell.HasNoCandidates())
	{
		return INDEX_NONE;
	}

	// select a candidate, applying weighted probabilities
	float TotalWeight = 0.f;
	TArray<float> TileWeights;
	for (const FWFCTileId& TileId : Cell.TileCandidates)
	{
		const float TileWeight = Model->GetTileWeightUnchecked(TileId);
		TileWeights.Add(TileWeight);
		TotalWeight += TileWeight;
	}

	if (FMath::IsNearlyZero(TotalWeight))
	{
		// no weights, treat all equally
		ThreadRandomStream.Initialize(int32(FPlatformTime::Cycles()));
		const int32 Idx = ThreadRandomStream.RandRange(0, Cell.TileCandidates.Num() - 1);

		return Cell.TileCandidates[Idx];
	}

	ThreadRandomStream.Initialize(int32(FPlatformTime::Cycles()));
	float Rand = ThreadRandomStream.GetFraction() * TotalWeight;
	for (int32 Idx = 0; Idx < Cell.TileCandidates.Num(); ++Idx)
	{
		if (Rand >= TileWeights[Idx])
		{
			Rand -= TileWeights[Idx];
		}
		else
		{
			return Cell.TileCandidates[Idx];
		}
	}

	return Cell.TileCandidates[0];
}

