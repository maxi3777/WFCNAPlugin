// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/AsyncConstraint.h"

class WFCNA_API FAsyncPositionConstraint : public FAsyncConstraint
{
public:
	FAsyncPositionConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel,
		TMap<FWFCCellIndex, TArray<FWFCTileId>> InTilesToBan, bool InDidApplyInitialConstraint);
	virtual ~FAsyncPositionConstraint() override;

	virtual void Reset() override;
	virtual bool Next() override;
	
	/** 从 UObject 预计算阶段传递过来的剔除黑名单：[CellIndex] -> [TileIds] */
	TMap<FWFCCellIndex, TArray<FWFCTileId>> TilesToBan;

	bool bDidApplyInitialConstraint;
	bool CacheDidApplyInitialConstraint;
};