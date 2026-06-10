// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISMManagerTypes.h"
#include "ISMInstanceBucket.generated.h"

class UInstancedStaticMeshComponent;

UCLASS()
class ISMMANAGER_API UISMInstanceBucket : public UObject
{
    GENERATED_BODY()

public:
    void InitializeBucket(AActor* InOwnerActor, const FISMBucketKey& InKey, int32 InInitialInstancesCount, int32 InComponentMaxInstancesCount);

    // ================== 服务器逻辑（管理池子） ==================
    TArray<int32> BatchAllocateInstances(const TArray<FTransform>& Transforms);
    void BatchFreeInstances(const TArray<int32>& Indices);

    // ================== 客户端逻辑（仅同步表现） ==================
    // 客户端直接根据 Server 下发的索引覆写 Transform，不维护 FreeIndices 的业务逻辑
    void ClientApplyTransforms(const TArray<int32>& Indices, const TArray<FTransform>& Transforms);
    void ClientFreeTransforms(const TArray<int32>& Indices);

    // ================== 状态获取 ==================
    const FISMBucketKey& GetBucketKey() const { return BucketKey; }
    
    // 获取当前真实存活（非空闲）的实例数量，用于 DataAsset 自动生成
    int32 GetActiveInstanceCount() const { return TotalInstanceCapacity - FreeIndices.Num(); }

private:
    // 位压缩：高12位为组件索引(最大4096)，低20位为实例索引(最大100万)
    static constexpr int32 COMPONENT_INDEX_BITS = 12;
    static constexpr int32 INSTANCE_INDEX_BITS = 20;
    static constexpr int32 INSTANCE_INDEX_MASK = (1 << INSTANCE_INDEX_BITS) - 1;

    FORCEINLINE int32 EncodeIndex(int32 CompIdx, int32 LocalIdx) const
    {
        return (CompIdx << INSTANCE_INDEX_BITS) | (LocalIdx & INSTANCE_INDEX_MASK);
    }

    FORCEINLINE void DecodeIndex(int32 GlobalIdx, int32& OutCompIdx, int32& OutLocalIdx) const
    {
        OutCompIdx = GlobalIdx >> INSTANCE_INDEX_BITS;
        OutLocalIdx = GlobalIdx & INSTANCE_INDEX_MASK;
    }

    void CreateAndAddNewComponent(int32 InstancesToCreate);

private:
    UPROPERTY()
    AActor* OwnerActor;

    UPROPERTY()
    FISMBucketKey BucketKey;

    int32 ComponentMaxInstancesCount;
    int32 TotalInstanceCapacity = 0; // 记录总容量

    UPROPERTY()
    TArray<UInstancedStaticMeshComponent*> AllComponents;

    // 全局空闲索引池（栈 LIFO）
    TArray<int32> FreeIndices;
};