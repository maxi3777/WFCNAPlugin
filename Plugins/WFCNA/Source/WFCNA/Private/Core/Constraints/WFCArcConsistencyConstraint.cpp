// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Constraints/WFCArcConsistencyConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/AsyncArcConsistencyConstraint.h"


void UWFCArcConstraintSnapshot::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << AllowedTiles;
	Ar << SupportCounts;
	Ar << DefaultSupportCounts;
	Ar << BansToPropagate;
}

UWFCArcConsistencyConstraint::UWFCArcConsistencyConstraint()
	: bIgnoreContradictionCells(false),
	  bIsInitialized(false),
	  bDidApplyInitialConsistency(false)
{
}

void UWFCArcConsistencyConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	if (bIsInitialized)
	{
		return;
	}

	bDidApplyInitialConsistency = false;
	BansToPropagate.Reset();

	// initialize allowed tiles to empty list for each combination of [tile][direction].
	AllowedTiles.AddZeroed(Model->GetNumTiles());
	for (int32 TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		AllowedTiles[TileId].AddZeroed(Grid->GetNumDirections());
	}

	// initialize support counts array (but don't fill it out or ban tiles yet)
	SupportCounts.Empty(Grid->GetNumCells());
	SupportCounts.AddZeroed(Grid->GetNumCells());
	for (int32 CellIndex = 0; CellIndex < Grid->GetNumCells(); ++CellIndex)
	{
		SupportCounts[CellIndex].AddZeroed(Model->GetNumTiles());
		for (int32 TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
		{
			SupportCounts[CellIndex][TileId].AddZeroed(Grid->GetNumDirections());
		}
	}

	// store for quick resetting
	DefaultSupportCounts = SupportCounts;

	bIsInitialized = true;
}

void UWFCArcConsistencyConstraint::Reset()
{
	Super::Reset();
	
	bDidApplyInitialConsistency = false;
	BansToPropagate.Reset();
	SupportCounts = DefaultSupportCounts;
}

void UWFCArcConsistencyConstraint::AddAllowedTileForDirection(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AllowedTileId)
{
	if (!AllowedTiles[TileId][Direction].Contains(AllowedTileId))
	{
		AllowedTiles[TileId][Direction].Add(AllowedTileId);
	}
}

const TArray<FWFCTileId>& UWFCArcConsistencyConstraint::GetAllowedTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const
{
	return AllowedTiles[TileId][Direction];
}

UWFCConstraintSnapshot* UWFCArcConsistencyConstraint::CreateSnapshot(UObject* Outer) const
{
	UWFCArcConstraintSnapshot* Snapshot = NewObject<UWFCArcConstraintSnapshot>(Outer);
	if (FAsyncArcConsistencyConstraint* AsyncArcConsistencyConstraint = static_cast<FAsyncArcConsistencyConstraint*>(AsyncConstraintForSnapshot))
	{
		Snapshot->AllowedTiles = AsyncArcConsistencyConstraint->AllowedTiles;
		Snapshot->SupportCounts = AsyncArcConsistencyConstraint->SupportCounts;
		Snapshot->DefaultSupportCounts = AsyncArcConsistencyConstraint->DefaultSupportCounts;
		Snapshot->BansToPropagate = AsyncArcConsistencyConstraint->BansToPropagate;
		return Snapshot;
	}
	UE_LOG(LogWFC, Error, TEXT("AsyncConstraintForSnapshot(UWFCArcConsistencyConstraint) not exist!"));
	return nullptr;
}

void UWFCArcConsistencyConstraint::ApplySnapshot(const UWFCConstraintSnapshot* Snapshot)
{
	const UWFCArcConstraintSnapshot* ArcSnapshot = Cast<UWFCArcConstraintSnapshot>(Snapshot);
	if (!ArcSnapshot)
	{
		return;
	}
	AllowedTiles = ArcSnapshot->AllowedTiles;
	SupportCounts = ArcSnapshot->SupportCounts;
	DefaultSupportCounts = ArcSnapshot->DefaultSupportCounts;
	BansToPropagate = ArcSnapshot->BansToPropagate;

	bIsInitialized = true;
	bDidApplyInitialConsistency = true;
}



void UWFCArcConsistencyConstraint::LogDebugInfo() const
{
	Super::LogDebugInfo();

	UE_LOG(LogWFC, Verbose, TEXT("%s AllowedTiles allocated size: %.3fKB"),
	       *GetClass()->GetName(), AllowedTiles.GetAllocatedSize() / 1024.f);
	UE_LOG(LogWFC, Verbose, TEXT("%s SupportCounts allocated size: %.3fKB"),
	       *GetClass()->GetName(), SupportCounts.GetAllocatedSize() / 1024.f);


	if (!Model)
	{
		return;
	}

	for (FWFCTileId TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		const FString TileStr = Model->GetTileDebugString(TileId);
		UE_LOG(LogWFC, VeryVerbose, TEXT("%s allowed tiles:"), *TileStr);

		const auto& AllowedDirections = AllowedTiles[TileId];
		for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
		{
			// log the opposite direction, since allowed tiles are stored as an 'incoming' direction
			// but it makes more sense to read this as the 'direction from this tile'
			const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);
			const FString DirectionStr = Grid->GetDirectionName(InvDirection);

			TArray<FString> TileStrs;
			const auto& ThisAllowedTiles = AllowedDirections[Direction];
			for (const int32& AllowedTileId : ThisAllowedTiles)
			{
				TileStrs.Add(FString::FromInt(AllowedTileId));
			}

			UE_LOG(LogWFC, VeryVerbose, TEXT("    %s: %s"), *DirectionStr, *FString::Join(TileStrs, TEXT(", ")));
		}
	}
}

TUniquePtr<FAsyncConstraint> UWFCArcConsistencyConstraint::CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel)
{
	return MakeUnique<FAsyncArcConsistencyConstraint>(InGenerator, InGrid, InModel,
		bIgnoreContradictionCells, bDidApplyInitialConsistency,
		AllowedTiles, SupportCounts, BansToPropagate);
}
