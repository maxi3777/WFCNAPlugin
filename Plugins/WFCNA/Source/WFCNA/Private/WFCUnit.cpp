// Copyright maxi3777. All Rights Reserved.


#include "WFCUnit.h"

#include "IxObjectPoolObject.h"
#include "WFCAsset.h"
#include "WFCModule.h"
#include "WFCStatics.h"
#include "WFCTileAsset.h"
#include "WFCUnitManager.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "ISMManager/Public/ISMManagerActor.h"


void UWFCUnit::Initialize()
{
	if (bIsPrimaryNode)
	{
		if (!ManagerClass || !ObjectPoolConfig || !ISMManagerConfig)
		{
			UE_LOG(LogWFC, Warning, TEXT("WFCUnit Settings were not specified"));
			return;
		}
		SelfManager = NewObject<UStackTreeManagerBase>(this, ManagerClass);
		UWFCUnitManager* UnitManager = Cast<UWFCUnitManager>(SelfManager);
		UnitManager->ObjectPool = NewObject<UIxObjectPool>(UnitManager, UIxObjectPool::StaticClass());
		UnitManager->ObjectPool->Initialize(ObjectPoolConfig);
		UnitManager->ISMManagerActor = GetWorld()->SpawnActor<AISMManagerActor>(AISMManagerActor::StaticClass());
		UnitManager->ISMManagerActor->Initialize(ISMManagerConfig, ISMMaxInstancesCount);
	}
}

void UWFCUnit::Run()
{
	Generator = UWFCStatics::CreateWFCGenerator(this, Assets[ActiveAssetIndex]);
	if (!Generator)
	{
		UE_LOG(LogWFC, Warning, TEXT("CreateWFCGenerator failed, consider no Asset"));
		return;
	}
	Generator->OnStateChanged.AddUObject(this, &UWFCUnit::OnWFCStateChanged);
	
	Generator->Initialize(false);
	if (bUseStartupSnapshot && Assets[ActiveAssetIndex]->StartupSnapshot)
	{
		Generator->ApplySnapshot(Assets[ActiveAssetIndex]->StartupSnapshot);
	}
	Generator->InitializeConstraints();
	
	Generator->StartCalculatingAsync();
}

void UWFCUnit::Construct()
{
	if (bIsPrimaryNode)
	{
		UWFCUnitManager* UnitManager = Cast<UWFCUnitManager>(SelfManager);
		if (!UnitManager){return;}
		if (ObjectPoolTransforms.Num() > 0)
		{
			for (auto ActorTransforms :ObjectPoolTransforms)
			{
				ObjectPoolMap.FindOrAdd(ActorTransforms.Key).Append(UnitManager->ObjectPool->GetActors(ActorTransforms.Key, ActorTransforms.Value));
			}
		}
		if (ISMTransforms.Num() > 0)
		{
			for (auto MeshTransforms:ISMTransforms)
			{
				ISMMangerMap.FindOrAdd(MeshTransforms.Key).Append(UnitManager->ISMManagerActor->BatchAllocateInstances(MeshTransforms.Key, MeshTransforms.Value));
			}
		}
	}
	else
	{
		UWFCUnitManager* UnitManager = Cast<UWFCUnitManager>(ParentManager);
		if (!UnitManager){return;}
		if (ObjectPoolTransforms.Num() > 0)
		{
			for (auto ActorTransforms :ObjectPoolTransforms)
			{
				ObjectPoolMap.FindOrAdd(ActorTransforms.Key).Append(UnitManager->ObjectPool->GetActors(ActorTransforms.Key, ActorTransforms.Value));
			}
		}
		if (ISMTransforms.Num() > 0)
		{
			for (auto MeshTransforms:ISMTransforms)
			{
				ISMMangerMap.FindOrAdd(MeshTransforms.Key).Append(UnitManager->ISMManagerActor->BatchAllocateInstances(MeshTransforms.Key, MeshTransforms.Value));
			}
		}
	}
}

void UWFCUnit::Destruct()
{
	if (bIsPrimaryNode)
	{
		UWFCUnitManager* UnitManager = Cast<UWFCUnitManager>(SelfManager);
		if (!UnitManager){return;}
		if (ObjectPoolMap.Num() > 0)
		{
			for (auto ActorHandle :ObjectPoolMap)
			{
				UnitManager->ObjectPool->ReturnActors(ActorHandle.Key, ActorHandle.Value);
			}
			ObjectPoolMap.Empty();
		}
		
		if (ISMMangerMap.Num() > 0)
		{
			for (auto ISMMangerHandle:ISMMangerMap)
			{
				UnitManager->ISMManagerActor->BatchFreeInstances(ISMMangerHandle.Key, ISMMangerHandle.Value);
			}
			ISMMangerMap.Empty();
		}
	}
	else
	{
		UWFCUnitManager* UnitManager = Cast<UWFCUnitManager>(ParentManager);
		if (!UnitManager){return;}
		if (ObjectPoolMap.Num() > 0)
		{
			for (auto ActorHandle :ObjectPoolMap)
			{
				UnitManager->ObjectPool->ReturnActors(ActorHandle.Key, ActorHandle.Value);
			}
			ObjectPoolMap.Empty();
		}
		
		if (ISMMangerMap.Num() > 0)
		{
			for (auto ISMMangerHandle:ISMMangerMap)
			{
				UnitManager->ISMManagerActor->BatchFreeInstances(ISMMangerHandle.Key, ISMMangerHandle.Value);
			}
			ISMMangerMap.Empty();
		}
	}
	ISMTransforms.Empty();
	ObjectPoolTransforms.Empty();
}

void UWFCUnit::OnWFCStateChanged(EWFCGeneratorState State)
{
	if (State == EWFCGeneratorState::Finished || State == EWFCGeneratorState::Error)
	{
		if (State == EWFCGeneratorState::Finished)
		{
			if (!Generator) return;
			for (int32 CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
			{
				const UWFCGrid* Grid = Generator->GetGrid();
				const FWFCModelAssetTile* AssetTile = GetAssetTileForCell(CellIndex);
				if (!AssetTile){return;}
				const UWFCTileAsset* TileAsset = Cast<UWFCTileAsset>(AssetTile->TileAsset.Get());
				if (!TileAsset){return;}
				const FTransform CellTransform = GetCellTransform(CellIndex, AssetTile->Rotation);
				const TArray<FActorInfoEntry> Actors = TileAsset->GetTileDefActorsInfo(AssetTile->TileDefIndex);
				const TArray<FStaticMeshInfoEntry> Meshes = TileAsset->GetTileDefMeshesInfo(AssetTile->TileDefIndex);
				const TSubclassOf<UWFCUnit> WFCUnit = TileAsset->GetTileDefUnit(AssetTile->TileDefIndex);
				if (Actors.Num() > 0)
				{
					for (auto ActorPair : Actors)
					{
						ObjectPoolTransforms.FindOrAdd(ActorPair.ActorClass).Add(CellTransform * ActorPair.Transform);
					}
				}
				if (Meshes.Num() > 0)
				{
					for (auto MeshPair : Meshes)
					{
						ISMTransforms.FindOrAdd(MeshPair.BucketKey).Add(CellTransform * MeshPair.Transform);
					}
				}
				if (WFCUnit != nullptr)
				{
					UWFCUnit* NewUnit = NewObject<UWFCUnit>(this, WFCUnit);
					AddChild(NewUnit);
					if (NewUnit->bIsUseForHexNesting)
					{
						FTransform TempTransform = CellTransform;
						FTransform RotateTransform(FRotator(0, 90, 0), FVector::ZeroVector, FVector::OneVector);
						TempTransform = RotateTransform * CellTransform;
						FVector WorldOffset =TempTransform.TransformVector(Grid->GetCenterOffsetToOrigin(UnitGridCenter));
						TempTransform.AddToTranslation(WorldOffset);
						NewUnit->GenRootTransform = TempTransform;
					}
					else
					{
						FTransform TempTransform = CellTransform;
						FVector WorldOffset =TempTransform.TransformVector(Grid->GetCenterOffsetToOrigin(UnitGridCenter));
						TempTransform.AddToTranslation(WorldOffset);
						NewUnit->GenRootTransform = TempTransform;
					}
					NewUnit->Initialize();
					NewUnit->Run();
				}
			}
			Preconstruct();
		}
		else
		{
			UE_LOG(LogWFC, Error, TEXT("WFCGenerator failed, Consider Asset (Not Sure): %s"), *(Assets[ActiveAssetIndex].GetName()));
		}
	}
}

void UWFCUnit::SetGenRootTransform(FTransform InTransform)
{
	GenRootTransform = InTransform;
}

const FWFCModelAssetTile* UWFCUnit::GetAssetTileForCell(int32 CellIndex) const
{
	if (!Generator || !Generator->IsValidCellIndex(CellIndex))
	{
		return nullptr;
	}

	const FWFCCell& Cell = Generator->GetCell(CellIndex);
	if (!Cell.HasSelection())
	{
		return nullptr;
	}

	const FWFCTileId SelectedTileId = Cell.GetSelectedTileId();
	if (!Generator->IsValidTileId(SelectedTileId))
	{
		return nullptr;
	}

	return Generator->GetModel()->GetTile<FWFCModelAssetTile>(SelectedTileId);
}

FTransform UWFCUnit::GetCellTransform(int32 CellIndex, int32 Rotation) const
{
	if (!Generator->IsInitialized())
	{
		return GenRootTransform;
	}

	const UWFCGrid* Grid = Generator->GetGrid();
	if (!Grid || !Grid->IsValidCellIndex(CellIndex))
	{
		return GenRootTransform;
	}

	FTransform CellTransform = Grid->GetCellWorldTransform(CellIndex, Rotation);
	FVector CombinedScale = GenRootTransform.GetScale3D() * CellTransform.GetScale3D();
	FVector CombinedTranslation = GenRootTransform.GetRotation().RotateVector(CellTransform.GetTranslation() * GenRootTransform.GetScale3D()) + GenRootTransform.GetTranslation();
	FQuat CombinedRotation = GenRootTransform.GetRotation() * CellTransform.GetRotation();
	return FTransform(CombinedRotation, CombinedTranslation, CombinedScale);
}
