// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Constraints/WFCPositionConstraint.h"

#include "WFCModule.h"
#include "WFCTileAsset.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCModel.h"
#include "Core/Grids/WFCGrid2D.h"
#include "Core/Grids/WFCGrid3D.h"
#include "Core/Grids/WFCGrid3DHex.h"
#include "Core/Constraints/AsyncPositionConstraint.h"


// ---------------------------------------------------------
// UWFCPositionConstraint (Base Class)
// ---------------------------------------------------------

void UWFCPositionConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	if (bIsInitialized)
	{
		return;
	}

	bDidApplyInitialConstraint = false;
	TilesToBan.Empty();
	
	// 在 UObject 初始化阶段 (主线程) 预计算所有格子的剔除列表，避免在异步线程访问 UObject 资产
	CalculateTilesToBan();

	bIsInitialized = true;
}

void UWFCPositionConstraint::Reset()
{
	Super::Reset();
	bDidApplyInitialConstraint = false;
}

UWFCConstraintSnapshot* UWFCPositionConstraint::CreateSnapshot(UObject* Outer) const
{
	UWFCPositionConstraintSnapshot* Snapshot = NewObject<UWFCPositionConstraintSnapshot>(Outer);
	if (FAsyncPositionConstraint* AsyncPosConstraint = static_cast<FAsyncPositionConstraint*>(AsyncConstraintForSnapshot))
	{
		Snapshot->bDidApplyInitialConstraint = AsyncPosConstraint->bDidApplyInitialConstraint;
		return Snapshot;
	}
	UE_LOG(LogWFC, Error, TEXT("AsyncConstraintForSnapshot(UWFCPositionConstraint) not exist!"));
	return nullptr;
}

void UWFCPositionConstraint::ApplySnapshot(const UWFCConstraintSnapshot* Snapshot)
{
	if (const UWFCPositionConstraintSnapshot* PosSnapshot = Cast<UWFCPositionConstraintSnapshot>(Snapshot))
	{
		bDidApplyInitialConstraint = PosSnapshot->bDidApplyInitialConstraint;
	}
}

TUniquePtr<FAsyncConstraint> UWFCPositionConstraint::CreateAsyncConstraint(FAsyncGenerator* InGenerator,
	TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel)
{
	// 将预计算的 TilesToBan 数据传递给纯 C++ 异步类
	return MakeUnique<FAsyncPositionConstraint>(InGenerator, InGrid, InModel, TilesToBan, bDidApplyInitialConstraint);
}

void UWFCPositionConstraint::GetTilesFailingRule(const FWFCPositionAxisRule& Rule, TArray<FWFCTileId>& OutFailingTiles) const
{
	OutFailingTiles.Empty();

	if (Rule.AllowedTags.IsEmpty() || !Model)
	{
		return;
	}

	for (FWFCTileId TileId = 0; TileId <= Model->GetMaxTileId(); ++TileId)
	{
		const FWFCModelAssetTile* AssetTile = Model->GetTile<FWFCModelAssetTile>(TileId);
		if (AssetTile && AssetTile->TileAsset.IsValid())
		{
			if (!Rule.AllowedTags.Matches(AssetTile->TileAsset->OwnedTags))
			{
				OutFailingTiles.Add(TileId);
			}
		}
	}
}


// ---------------------------------------------------------
// UWFCPosition2DConstraint
// ---------------------------------------------------------

void UWFCPosition2DConstraint::CalculateTilesToBan()
{
	const UWFCGrid2D* Grid2D = Cast<UWFCGrid2D>(Grid);
	if (!Grid2D)
	{
		UE_LOG(LogWFC, Error, TEXT("UWFCPosition2DConstraint: UWFCGrid2D can not be found"));
		return;
	}

	TArray<FWFCTileId> FailingX;
	TArray<FWFCTileId> FailingY;
	GetTilesFailingRule(RuleX, FailingX);
	GetTilesFailingRule(RuleY, FailingY);

	if (FailingX.IsEmpty() && FailingY.IsEmpty()) return;

	for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
	{
		const FIntPoint GridLocation = Grid2D->GetLocationForCellIndex(CellIndex);
		TArray<FWFCTileId> BansForThisCell;

		if (GridLocation.X >= RuleX.Range.X && GridLocation.X <= RuleX.Range.Y)
		{
			for (FWFCTileId FailingId : FailingX) BansForThisCell.AddUnique(FailingId);
		}
		
		if (GridLocation.Y >= RuleY.Range.X && GridLocation.Y <= RuleY.Range.Y)
		{
			for (FWFCTileId FailingId : FailingY) BansForThisCell.AddUnique(FailingId);
		}

		if (BansForThisCell.Num() > 0)
		{
			TilesToBan.Add(CellIndex, BansForThisCell);
		}
	}
}


// ---------------------------------------------------------
// UWFCPosition3DConstraint
// ---------------------------------------------------------

void UWFCPosition3DConstraint::CalculateTilesToBan()
{
	const UWFCGrid3D* Grid3D = Cast<UWFCGrid3D>(Grid);
	const UWFCGrid3DHex* GridHex = Cast<UWFCGrid3DHex>(Grid);

	if (!Grid3D && !GridHex)
	{
		UE_LOG(LogWFC, Error, TEXT("UWFCPosition3DConstraint: UWFCGrid3D/3DHex can not be found"));
		return;
	}

	TArray<FWFCTileId> FailingX;
	TArray<FWFCTileId> FailingY;
	TArray<FWFCTileId> FailingZ;
	GetTilesFailingRule(RuleX, FailingX);
	GetTilesFailingRule(RuleY, FailingY);
	GetTilesFailingRule(RuleZ, FailingZ);

	if (FailingX.IsEmpty() && FailingY.IsEmpty() && FailingZ.IsEmpty()) return;

	for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
	{
		FIntVector GridLocation = Grid3D ? Grid3D->GetLocationForCellIndex(CellIndex) : GridHex->GetLocationForCellIndex(CellIndex);
		TArray<FWFCTileId> BansForThisCell;

		if (GridLocation.X >= RuleX.Range.X && GridLocation.X <= RuleX.Range.Y)
		{
			for (FWFCTileId FailingId : FailingX) BansForThisCell.AddUnique(FailingId);
		}
		
		if (GridLocation.Y >= RuleY.Range.X && GridLocation.Y <= RuleY.Range.Y)
		{
			for (FWFCTileId FailingId : FailingY) BansForThisCell.AddUnique(FailingId);
		}

		if (GridLocation.Z >= RuleZ.Range.X && GridLocation.Z <= RuleZ.Range.Y)
		{
			for (FWFCTileId FailingId : FailingZ) BansForThisCell.AddUnique(FailingId);
		}

		if (BansForThisCell.Num() > 0)
		{
			TilesToBan.Add(CellIndex, BansForThisCell);
		}
	}
}