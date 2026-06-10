// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026


#include "Core/Constraints/WFCCountConstraint.h"

#include "WFCAssetModel.h"
#include "WFCModule.h"
#include "WFCTileAsset.h"
#include "Core/WFCGenerator.h"
#include "Core/Constraints/AsyncCountConstraint.h"


// UWFCCountConstraint
// -------------------

void UWFCCountConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);
}

void UWFCCountConstraint::Reset()
{
	Super::Reset();

	TileGroupCurrentCounts.Reset(TileGroupMaxCounts.Num());
	TileGroupCurrentCounts.SetNum(TileGroupMaxCounts.Num());
}

void UWFCCountConstraint::AddTileGroupMaxCountMapping(const TArray<FWFCTileId>& TileIds, int32 MaxCount)
{
	if (MaxCount <= 0)
	{
		UE_LOG(LogWFC, Warning, TEXT("MaxCount must be > 0 for a count constraint: %s"),
		       *GetNameSafe(GetOuter()));
		return;
	}

	// add tile group and max count
	const int32 GroupId = TileGroupMaxCounts.Add(FWFCCountConstraintTileGroup(TileIds, MaxCount));
	TileGroupCurrentCounts.SetNum(TileGroupMaxCounts.Num());

	// cache tile id -> group id mappings
	for (const FWFCTileId& TileId : TileIds)
	{
		TileIdsToGroups.Add(TileId, GroupId);
	}

	UE_LOG(LogWFC, VeryVerbose, TEXT("UWFCCountConstraint: Setting Max Count of %d for %d tile(s)"),
	       MaxCount, TileIds.Num());
}

TUniquePtr<FAsyncConstraint> UWFCCountConstraint::CreateAsyncConstraint(FAsyncGenerator* InGenerator,
	TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel)
{
	return MakeUnique<FAsyncCountConstraint>(InGenerator, InGrid, InModel, TileGroupMaxCounts, TileIdsToGroups, TileGroupCurrentCounts);
}


// UWFCTagCountConstraint
// ----------------------

int32 UWFCTagCountConstraint::GetTileMaxCount(const UWFCTileAsset* TileAsset) const
{
	if (!TileAsset)
	{
		return 0;
	}
	const FWFCTileTagMaxCount* MaxCountRule = MaxCounts.FindByPredicate([TileAsset](const FWFCTileTagMaxCount& MaxCountRule)
	{
		return TileAsset->OwnedTags.HasTag(MaxCountRule.Tag);
	});

	if (MaxCountRule)
	{
		return MaxCountRule->MaxCount;
	}
	return 0;
}

void UWFCTagCountConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	const UWFCAssetModel* AssetModel = Cast<UWFCAssetModel>(Model);
	if (!AssetModel)
	{
		UE_LOG(LogWFC, Error, TEXT("%s requires a UWFCAssetModel: %s"), *GetClass()->GetName(), *GetNameSafe(GetOuter()));
		return;
	}

	TArray<UWFCTileAsset*> TileAssets;
	AssetModel->GetAllTileAssets(TileAssets);

	for (const UWFCTileAsset* TileAsset : TileAssets)
	{
		const int32 TileMaxCount = GetTileMaxCount(TileAsset);
		if (TileMaxCount > 0)
		{
			const TArray<FWFCTileId> IdArray = AssetModel->GetTileIdsForAsset(TileAsset);

			// only apply the max count limitation to the tile at 0,0,0 within the asset,
			// so that large tiles don't count against it multiple times.
			TArray<FWFCTileId> OriginTileIds = IdArray.FilterByPredicate([AssetModel](const int32& TileId)
			{
				const FWFCModelAssetTile* AssetTile = AssetModel->GetTile<FWFCModelAssetTile>(TileId);
				check(AssetTile != nullptr);
				return AssetTile->TileDefIndex == 0;
			});

			if (OriginTileIds.Num() > 0)
			{
				AddTileGroupMaxCountMapping(OriginTileIds, TileMaxCount);
			}
		}
	}

	UE_LOG(LogWFC, Verbose, TEXT("UWFCTileAssetCountConstraintConfig configured %d max count mappings"), TileGroupMaxCounts.Num());
}

TUniquePtr<FAsyncConstraint> UWFCTagCountConstraint::CreateAsyncConstraint(FAsyncGenerator* InGenerator,
	TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel)
{
	return Super::CreateAsyncConstraint(InGenerator, InGrid, InModel);
}
