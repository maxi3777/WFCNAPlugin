// Copyright maxi3777. All Rights Reserved.


#include "Core/CellSelectors/AsyncEntropyCellSelector.h"

#include "Core/AsyncGenerator.h"
#include "Core/AsyncModel.h"

FAsyncEntropyCellSelector::FAsyncEntropyCellSelector(FAsyncGenerator* InAsyncGenerator, float InRandomDeviation)
	: FAsyncCellSelector(InAsyncGenerator), RandomDeviation(InRandomDeviation)
{
}

FAsyncEntropyCellSelector::~FAsyncEntropyCellSelector()
{
}

FWFCCellIndex FAsyncEntropyCellSelector::SelectNextCell()
{
	check(AsyncGenerator != nullptr);

	float MinEntropy = MAX_FLT;
	FWFCCellIndex BestCellIndex = INDEX_NONE;

	// select the cell with the lowest entropy, adding in a bit of randomness
	for (FWFCCellIndex CellIndex = 0; CellIndex < AsyncGenerator->GetNumCells(); ++CellIndex)
	{
		const FWFCCell& Cell = AsyncGenerator->GetCell(CellIndex);

		if (Cell.HasSelectionOrNoCandidates())
		{
			// nothing to collapse
			continue;
		}
		//线程安全随机
		ThreadRandomStream.Initialize(int32(FPlatformTime::Cycles()));
		const float Entropy = CalculateShannonEntropy(Cell) + ThreadRandomStream.GetFraction() * RandomDeviation;
		if (Entropy < MinEntropy)
		{
			MinEntropy = Entropy;
			BestCellIndex = CellIndex;
		}
	}

	return BestCellIndex;
}

float FAsyncEntropyCellSelector::CalculateShannonEntropy(const FWFCCell& Cell) const
{
	check(AsyncGenerator != nullptr);

	float SumOfWeights = 0.f;
	float SumOfLogWeights = 0.f;

	for (const FWFCTileId& TileId : Cell.TileCandidates)
	{
		const float Weight = AsyncGenerator->GetModel()->GetTileWeightUnchecked(TileId);
		if (Weight <= 0.f)
		{
			continue;
		}

		SumOfWeights += Weight;
		SumOfLogWeights += Weight * FMath::Loge(Weight);
	}

	return FMath::Loge(SumOfWeights) - (SumOfLogWeights / SumOfWeights);
}