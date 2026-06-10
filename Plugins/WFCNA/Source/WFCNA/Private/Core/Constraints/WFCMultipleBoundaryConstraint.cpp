// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Constraints/WFCMultipleBoundaryConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Grids/WFCGrid3D.h"
#include "Core/Grids/WFCGrid3DHex.h"
#include "Core/Constraints/AsyncBoundaryConstraint.h" // 完美复用此异步类

void UWFCMultipleBoundaryConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	if (bIsInitialized)
	{
		return;
	}

	bDidApplyInitialConstraint = false;
	const int32 NumDirections = Grid->GetNumDirections();

	// 遍历所有 Tile，计算它们在各个方向上是否允许靠边
	for (FWFCTileId TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		const FWFCModelAssetTile& Tile = Model->GetTileRef<FWFCModelAssetTile>(TileId);

		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			if (!CanTileBeNextToBoundary(Tile, Direction))
			{
				AddProhibitedAdjacentBoundaryMapping(TileId, Direction);
			}
		}
	}

	bIsInitialized = true;
}

void UWFCMultipleBoundaryConstraint::Reset()
{
	Super::Reset();
	bDidApplyInitialConstraint = false;
}

void UWFCMultipleBoundaryConstraint::AddProhibitedAdjacentBoundaryMapping(FWFCTileId TileId, FWFCGridDirection Direction)
{
	TileBoundaryProhibitionMap.FindOrAdd(TileId).AddUnique(Direction);
}

bool UWFCMultipleBoundaryConstraint::CanTileBeNextToBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction) const
{
	const UWFCTileAsset* TileAsset = Tile.TileAsset.Get();
	if (!TileAsset)
	{
		return false;
	}

	// 转换为局部方向，以读取 TileAsset 定义的 EdgeType
	const FWFCGridDirection LocalDirection = Grid->InverseRotateDirection(Direction, Tile.Rotation);

	if (TileAsset->IsInteriorEdge(Tile.TileDefIndex, LocalDirection))
	{
		// 大型 Tile 内部的边永远不能贴着网格边界
		return false;
	}

	const FGameplayTag EdgeType = TileAsset->GetTileDefEdgeType(Tile.TileDefIndex, LocalDirection);
	const FGameplayTagContainer EdgeTypeTags(EdgeType);

	// 判定方向类别
	bool bIsTop = false;
	bool bIsBottom = false;

	// 仅 3D 网格区分上下 (最后两个 Direction)
	if (Grid->IsA<UWFCGrid3D>() || Grid->IsA<UWFCGrid3DHex>())
	{
		const int32 NumDirections = Grid->GetNumDirections();
		if (Direction == NumDirections - 2)
		{
			bIsTop = true;
		}
		else if (Direction == NumDirections - 1)
		{
			bIsBottom = true;
		}
	}

	// 选择使用的 Query
	const FGameplayTagQuery* QueryToUse = &HorizontalEdgeTypeQuery;
	if (bIsTop)
	{
		QueryToUse = &TopEdgeTypeQuery;
	}
	else if (bIsBottom)
	{
		QueryToUse = &BottomEdgeTypeQuery;
	}

	// 检查选中的 Query 是否对标签进行了限制
	if (!QueryToUse->IsEmpty() && !QueryToUse->Matches(EdgeTypeTags))
	{
		return false;
	}

	return true;
}

UWFCConstraintSnapshot* UWFCMultipleBoundaryConstraint::CreateSnapshot(UObject* Outer) const
{
	UWFCMultipleBoundaryConstraintSnapshot* Snapshot = NewObject<UWFCMultipleBoundaryConstraintSnapshot>(Outer);
	if (FAsyncBoundaryConstraint* AsyncBoundary = static_cast<FAsyncBoundaryConstraint*>(AsyncConstraintForSnapshot))
	{
		Snapshot->bDidApplyInitialConstraint = AsyncBoundary->bDidApplyInitialConstraint;
		return Snapshot;
	}
	UE_LOG(LogWFC, Error, TEXT("AsyncConstraintForSnapshot(UWFCMultipleBoundaryConstraint) not exist!"));
	return nullptr;
}

void UWFCMultipleBoundaryConstraint::ApplySnapshot(const UWFCConstraintSnapshot* Snapshot)
{
	if (const UWFCMultipleBoundaryConstraintSnapshot* MultipleSnapshot = Cast<UWFCMultipleBoundaryConstraintSnapshot>(Snapshot))
	{
		bDidApplyInitialConstraint = MultipleSnapshot->bDidApplyInitialConstraint;
	}
}

TUniquePtr<FAsyncConstraint> UWFCMultipleBoundaryConstraint::CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel)
{
	return MakeUnique<FAsyncBoundaryConstraint>(InGenerator, InGrid, InModel, TileBoundaryProhibitionMap, bDidApplyInitialConstraint);
}