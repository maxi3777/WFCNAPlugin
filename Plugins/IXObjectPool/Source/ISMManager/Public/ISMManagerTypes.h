// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ISMManagerTypes.generated.h"

/**
 * 核心哈希键：用于唯一标识一类 ISM/HISM 实例桶
 * 只要这四个属性完全一致，就会被分配到同一个 Bucket 中进行批处理
 */
USTRUCT(BlueprintType)
struct ISMMANAGER_API FISMBucketKey
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISM Key")
    bool bUseHISM = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISM Key")
    UStaticMesh* StaticMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISM Key")
    UMaterialInterface* Material = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISM Key")
    FName CollisionProfile = NAME_None;

    // 重载判等运算符，供 TMap 内部解决哈希冲突使用
    bool operator==(const FISMBucketKey& Other) const
    {
        return bUseHISM == Other.bUseHISM &&
               StaticMesh == Other.StaticMesh &&
               Material == Other.Material &&
               CollisionProfile == Other.CollisionProfile;
    }
};

/**
 * 全局哈希函数，使 FISMBucketKey 可以直接作为 TMap 的 Key
 * 采用 UE 提供的 HashCombine 混合哈希值，确保散列均匀
 */
FORCEINLINE uint32 GetTypeHash(const FISMBucketKey& Key)
{
    uint32 Hash = FCrc::MemCrc32(&Key.bUseHISM, sizeof(bool));
    Hash = HashCombine(Hash, GetTypeHash(Key.StaticMesh));
    Hash = HashCombine(Hash, GetTypeHash(Key.Material));
    Hash = HashCombine(Hash, GetTypeHash(Key.CollisionProfile));
    return Hash;
}

/**
 * DataAsset 中的单行配置
 */
USTRUCT(BlueprintType)
struct ISMMANAGER_API FISMConfigRow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISM Config")
    FISMBucketKey BucketKey;

    // 根据 DataAsset 初始创建的 HISM/ISM 所包含的默认实例数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISM Config", meta=(ClampMin="1"))
    int32 InstancesCount = 1000; 
};

/**
 * 驱动整个 ISM/HISM 管理系统的核心数据资产 (DataAsset)
 * 可在编辑器中手动配置，也可由系统的 Editor 功能自动扫描生成
 */
UCLASS(BlueprintType)
class ISMMANAGER_API UISMManagerConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ISM Configuration")
    TArray<FISMConfigRow> ConfigRows;
};