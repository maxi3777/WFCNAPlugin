// Copyright maxi3777. All Rights Reserved.


#include "Core/Constraints/AsyncPositionConstraint.h"
#include "Core/AsyncGenerator.h"

FAsyncPositionConstraint::FAsyncPositionConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
	TMap<FWFCCellIndex, TArray<FWFCTileId>> InTilesToBan, bool InDidApplyInitialConstraint)
	: FAsyncConstraint(InGenerator, InGrid, InModel),
	  TilesToBan(InTilesToBan),
	  bDidApplyInitialConstraint(InDidApplyInitialConstraint),
	  CacheDidApplyInitialConstraint(InDidApplyInitialConstraint)
{
}

FAsyncPositionConstraint::~FAsyncPositionConstraint()
{
}

void FAsyncPositionConstraint::Reset()
{
	FAsyncConstraint::Reset();
	bDidApplyInitialConstraint = CacheDidApplyInitialConstraint;
}

bool FAsyncPositionConstraint::Next()
{
	if (bDidApplyInitialConstraint)
	{
		return false; // 只在初始化时执行一次
	}

	bool bDidMakeChanges = false;

	// 应用预计算好的剔除列表
	if (!TilesToBan.IsEmpty())
	{
		for (const auto& Elem : TilesToBan)
		{
			if (Generator->BanMultiple(Elem.Key, Elem.Value))
			{
				// 发生矛盾 (Contradiction)
				return true; 
			}
		}
		bDidMakeChanges = true;
	}

	bDidApplyInitialConstraint = true;
	return bDidMakeChanges;
}