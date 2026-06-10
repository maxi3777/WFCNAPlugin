// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AsyncCellSelector.h"


FAsyncCellSelector::FAsyncCellSelector(FAsyncGenerator* InAsyncGenerator) : AsyncGenerator(InAsyncGenerator)
{
}

FAsyncCellSelector::~FAsyncCellSelector()
{
}

FWFCCellIndex FAsyncCellSelector::SelectNextCell()
{
	return INDEX_NONE;
}
