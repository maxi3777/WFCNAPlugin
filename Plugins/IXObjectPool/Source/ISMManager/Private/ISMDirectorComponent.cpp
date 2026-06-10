// Copyright maxi3777. All Rights Reserved.

#include "ISMDirectorComponent.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#endif

UISMDirectorComponent::UISMDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 追求极致性能，不使用Tick
    SetIsReplicatedByDefault(true);
    bWantsInitializeComponent = true;
}

void UISMDirectorComponent::InitializeComponent()
{
    Super::InitializeComponent();

    // 仅在服务器或单机模式下基于 DataAsset 预先分配
    if (GetOwner()->HasAuthority() && ConfigurationAsset)
    {
        for (const FISMConfigRow& Row : ConfigurationAsset->ConfigRows)
        {
            GetOrCreateBucket(Row.BucketKey, Row.InstancesCount);
        }
    }
}

UISMInstanceBucket* UISMDirectorComponent::GetOrCreateBucket(const FISMBucketKey& Key, int32 InitialInstancesCount)
{
    if (UISMInstanceBucket** FoundBucket = BucketPools.Find(Key))
    {
        return *FoundBucket;
    }

    UISMInstanceBucket* NewBucket = NewObject<UISMInstanceBucket>(this);
    NewBucket->InitializeBucket(GetOwner(), Key, InitialInstancesCount, ComponentMaxInstancesCount);
    BucketPools.Add(Key, NewBucket);
    return NewBucket;
}

TArray<int32> UISMDirectorComponent::BatchAllocateInstances(const FISMBucketKey& Key, const TArray<FTransform>& Transforms)
{
    if (Transforms.IsEmpty()) return TArray<int32>();

    if (!GetOwner()->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("BatchAllocateInstances must be called on Server!"));
        return TArray<int32>();
    }

    UISMInstanceBucket* Bucket = GetOrCreateBucket(Key);
    TArray<int32> AllocatedIndices = Bucket->BatchAllocateInstances(Transforms);

    if (GetNetMode() != NM_Standalone)
    {
        Multicast_BatchAllocate(Key, AllocatedIndices, Transforms);
    }

    return AllocatedIndices;
}

void UISMDirectorComponent::BatchFreeInstances(const FISMBucketKey& Key, const TArray<int32>& Indices)
{
    if (Indices.IsEmpty()) return;

    if (!GetOwner()->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("BatchFreeInstances must be called on Server!"));
        return;
    }

    UISMInstanceBucket* Bucket = GetOrCreateBucket(Key);
    Bucket->BatchFreeInstances(Indices);

    if (GetNetMode() != NM_Standalone)
    {
        Multicast_BatchFree(Key, Indices);
    }
}

void UISMDirectorComponent::Multicast_BatchAllocate_Implementation(const FISMBucketKey& Key, const TArray<int32>& AllocatedIndices, const TArray<FTransform>& Transforms)
{
    // 拦截 Server，因为 Server 已经在上面的主函数中处理过了，防止重复执行
    if (GetOwner()->HasAuthority() && GetNetMode() != NM_Client) return;

    UISMInstanceBucket* Bucket = GetOrCreateBucket(Key, 0); // 客户端如果没这个桶就建一个
    Bucket->ClientApplyTransforms(AllocatedIndices, Transforms);
}

void UISMDirectorComponent::Multicast_BatchFree_Implementation(const FISMBucketKey& Key, const TArray<int32>& Indices)
{
    if (GetOwner()->HasAuthority() && GetNetMode() != NM_Client) return;

    if (UISMInstanceBucket** BucketPtr = BucketPools.Find(Key))
    {
        // 客户端在拿到乱序的销毁请求时，也要排序成连续块提高显存写入效率
        TArray<int32> SortedIndices = Indices;
        SortedIndices.Sort();
        (*BucketPtr)->ClientFreeTransforms(SortedIndices);
    }
}

#if WITH_EDITOR
void UISMDirectorComponent::GenerateOrUpdateDataAsset()
{
    FString PackageName = DataAssetSavePath;
    UPackage* Package = CreatePackage(*PackageName);
    Package->FullyLoad();

    UISMManagerConfig* ConfigAsset = FindObject<UISMManagerConfig>(Package, *FPaths::GetBaseFilename(DataAssetSavePath));

    if (!ConfigAsset)
    {
        ConfigAsset = NewObject<UISMManagerConfig>(Package, UISMManagerConfig::StaticClass(), *FPaths::GetBaseFilename(DataAssetSavePath), RF_Public | RF_Standalone);
    }
    else
    {
        ConfigAsset->Modify();
    }

    for (auto& Pair : BucketPools)
    {
        const FISMBucketKey& Key = Pair.Key;
        // 获取实际使用了多少实例（总容量 - 空闲数）
        int32 ActiveCount = Pair.Value->GetActiveInstanceCount(); 

        FISMConfigRow* ExistingRow = ConfigAsset->ConfigRows.FindByPredicate([&](const FISMConfigRow& Row) {
            return Row.BucketKey == Key;
        });

        if (ExistingRow)
        {
            if (ExistingRow->InstancesCount < ActiveCount)
            {
                ExistingRow->InstancesCount = ActiveCount;
            }
        }
        else
        {
            FISMConfigRow NewRow;
            NewRow.BucketKey = Key;
            NewRow.InstancesCount = ActiveCount;
            ConfigAsset->ConfigRows.Add(NewRow);
        }
    }

    ConfigAsset->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(ConfigAsset);

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    UPackage::SavePackage(Package, ConfigAsset, *FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension()), SaveArgs);

    UE_LOG(LogTemp, Log, TEXT("ISM DataAsset updated successfully at: %s"), *DataAssetSavePath);
}
#endif