// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026


#include "WFCGeneratorComponent.h"

#include "WFCAsset.h"
#include "WFCModule.h"
#include "WFCStatics.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "GameFramework/Actor.h"


UWFCGeneratorComponent::UWFCGeneratorComponent()
	: StepLimit(100000),
	  MaxRetry(50),
	  bUseStartupSnapshot(true),
	  bAutoRun(true),
	  DebugGridColor(FLinearColor::White)
{
}

void UWFCGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRun)
	{
		Run();
	}
}

bool UWFCGeneratorComponent::Initialize(bool bForce)
{
	if (!bForce && IsInitialized())
	{
		// already initialized
		return true;
	}

	if (!WFCAsset)
	{
		UE_LOG(LogWFC, Warning, TEXT("No WFCAsset was specified: %s"), *GetNameSafe(GetOwner()));
		return false;
	}

	Generator = UWFCStatics::CreateWFCGenerator(this, WFCAsset);
	if (!Generator)
	{
		return false;
	}
	
	Generator->OnStateChanged.AddUObject(this, &UWFCGeneratorComponent::OnStateChanged);

	Generator->Initialize(false);

	if (bUseStartupSnapshot && WFCAsset->StartupSnapshot)
	{
		Generator->ApplySnapshot(WFCAsset->StartupSnapshot);
	}

	Generator->InitializeConstraints();

	return true;
}

bool UWFCGeneratorComponent::IsInitialized() const
{
	return Generator && Generator->IsInitialized();
}

void UWFCGeneratorComponent::ResetGenerator()
{
	Initialize(true);
}

void UWFCGeneratorComponent::Run()
{
	if (!IsInitialized())
	{
		Initialize();
	}

	if (IsInitialized())
	{
		Generator->StartCalculatingAsync(MaxRetry, StepLimit);
	}
}

void UWFCGeneratorComponent::RunAgain()
{
	if (!IsInitialized())
	{
		Initialize();
	}
	
	if (IsInitialized())
	{
		Generator->ResetForRunAgain();
		Generator->StartCalculatingAsync(MaxRetry, StepLimit);
	}
}

const UWFCGrid* UWFCGeneratorComponent::GetGrid() const
{
	return Generator ? Generator->GetGrid() : nullptr;
}

EWFCGeneratorState UWFCGeneratorComponent::GetState() const
{
	return Generator ? Generator->State : EWFCGeneratorState::None;
}

void UWFCGeneratorComponent::GetSelectedTileId(int32 CellIndex, bool& bSuccess, int32& TileId) const
{
	TileId = INDEX_NONE;
	bSuccess = false;
	if (Generator)
	{
		const FWFCCell& Cell = Generator->GetCell(CellIndex);
		if (Cell.HasSelection())
		{
			TileId = Cell.GetSelectedTileId();
			bSuccess = true;
		}
	}
}

void UWFCGeneratorComponent::OnCellSelected(int32 CellIndex)
{
	OnCellSelectedEvent.Broadcast(CellIndex);
	OnCellSelectedEvent_BP.Broadcast(CellIndex);
}

void UWFCGeneratorComponent::OnStateChanged(EWFCGeneratorState State)
{
	OnStateChangedEvent_BP.Broadcast(State);

	if (State == EWFCGeneratorState::Finished || State == EWFCGeneratorState::Error)
	{
		if (State == EWFCGeneratorState::Finished)
		{
			UE_LOG(LogWFC, Log, TEXT("WFCGenerator finished successfully: %s"), *GetOwner()->GetName());
		}
		else
		{
			UE_LOG(LogWFC, Error, TEXT("WFCGenerator failed: %s"), *GetOwner()->GetName());
		}

		OnFinishedEvent_BP.Broadcast(State == EWFCGeneratorState::Finished);
		OnFinishedEvent.Broadcast(State);
	}
}
