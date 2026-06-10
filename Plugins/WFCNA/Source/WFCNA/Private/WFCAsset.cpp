// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCAsset.h"

#include "WFCAssetModel.h"
#include "WFCStatics.h"
#include "WFCTileSet.h"
#include "Core/WFCCellSelector.h"
#include "Core/WFCGenerator.h"
#include "Core/CellSelectors/WFCEntropyCellSelector.h"
#include "Misc/DataValidation.h"
#include "UObject/ObjectSaveContext.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "WFCEditor"

UWFCAsset::UWFCAsset()
{
	GeneratorClass = UWFCGenerator::StaticClass();
	CellSelectorClass = UWFCEntropyCellSelector::StaticClass();
	ModelClass = UWFCAssetModel::StaticClass();
}

#if WITH_EDITOR
void UWFCAsset::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	if (!SaveContext.IsProceduralSave())
	{
		// remove invalid entries
		TileSets.RemoveAll([](const UWFCTileSet* TileSet)
		{
			return TileSet == nullptr;
		});

		// sort tile sets by name
		TileSets.Sort([](const UWFCTileSet& TileSetA, const UWFCTileSet& TileSetB)
		{
			return TileSetA.GetName() < TileSetB.GetName();
		});
	}
}

EDataValidationResult UWFCAsset::IsDataValid(FDataValidationContext& Context)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (TileSets.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

void UWFCAsset::UpdateSnapshot()
{
	StartupSnapshot = nullptr;

	UWFCGenerator* Generator = UWFCStatics::CreateWFCGenerator(GetTransientPackage(), this);
	if (!Generator)
	{
		return;
	}

	Generator->Initialize();
	StartupSnapshot = Generator->SyncStartupAndCreateSnapshot(this);
	if (Generator->State == EWFCGeneratorState::Error)
	{
		UE_LOG(LogGameplayTags, Error, TEXT("Fail to updateSnapshot, generator error"));
		return;
	}
	Modify();
}

#endif

#undef LOCTEXT_NAMESPACE
