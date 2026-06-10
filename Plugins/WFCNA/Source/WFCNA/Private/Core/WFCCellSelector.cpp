// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/WFCCellSelector.h"

#include "Core/AsyncCellSelector.h"


// UWFCCellSelector
// ----------------

void UWFCCellSelector::Initialize()
{
}

void UWFCCellSelector::Reset()
{
}

TUniquePtr<FAsyncCellSelector> UWFCCellSelector::CreateAsyncCellSelector(FAsyncGenerator* InAsyncGenerator)
{
	return MakeUnique<FAsyncCellSelector>(InAsyncGenerator);
}



