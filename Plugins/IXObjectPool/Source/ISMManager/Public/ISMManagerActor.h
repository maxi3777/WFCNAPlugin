// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ISMManagerTypes.h"
#include "ISMInstanceBucket.h"
#include "ISMManagerActor.generated.h"

/**
 * 专用于网络同步初始化容量的数据结构
 */
USTRUCT()
struct FISMInitData
{
    GENERATED_BODY()

    UPROPERTY()
    FISMBucketKey Key;

    UPROPERTY()
    int32 InitialCount = 0;
};

/**
 * ISM 实例的全局管控 Actor
 * 负责维护所有 Bucket，处理基于栈式分配的批处理，并负责将视觉表现完美同步给所有客户端。
 */
UCLASS()
class ISMMANAGER_API AISMManagerActor : public AActor
{
    GENERATED_BODY()
    
public:    
    AISMManagerActor();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // ==========================================
    // 生命周期与初始化 (仅服务器)
    // ==========================================

    /**
     * 初始化系统
     * @param Config - 包含各个 Bucket 初始预分配容量的数据资产
     * @param MaxInstancesCount - 当需要动态扩容时，每个新 ISM/HISM 组件包含的最大实例数
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "ISM Manager")
    void Initialize(UISMManagerConfig* Config, int32 MaxInstancesCount);

    // ==========================================
    // 公开运行时 API (仅服务器调用)
    // ==========================================

    /**
     * 批量分配/创建实例
     * @param Key - 实例的特征键
     * @param Transforms - 需要分配的初始 Transform 数组
     * @return 分配到的全局压缩索引数组（供后续销毁使用）
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "ISM Manager")
    TArray<int32> BatchAllocateInstances(const FISMBucketKey& Key, const TArray<FTransform>& Transforms);

    /**
     * 批量释放/销毁实例（软删除）
     * @param Key - 实例的特征键
     * @param Indices - 之前分配到的压缩索引数组
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "ISM Manager")
    void BatchFreeInstances(const FISMBucketKey& Key, const TArray<int32>& Indices);

private:
    // 获取或创建对应的 Bucket
    UISMInstanceBucket* GetOrCreateBucket(const FISMBucketKey& Key, int32 InitialInstancesCount = 50);

    // ==========================================
    // 网络同步变量 (解决中途加入玩家的容量对齐问题)
    // ==========================================

    // 动态扩容阈值，同步给客户端
    UPROPERTY(Replicated)
    int32 ComponentMaxInstancesCount = 50;

    // 初始配置数据，同步给客户端触发 OnRep 创建对应的初始 Bucket
    UPROPERTY(ReplicatedUsing = OnRep_BucketInitData)
    TArray<FISMInitData> BucketInitData;

    UFUNCTION()
    void OnRep_BucketInitData();

    // ==========================================
    // 网络同步 RPC (多端表现同步)
    // ==========================================

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BatchAllocate(const FISMBucketKey& Key, const TArray<int32>& AllocatedIndices, const TArray<FTransform>& Transforms);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BatchFree(const FISMBucketKey& Key, const TArray<int32>& Indices);

private:
    // 所有的对象池桶
    UPROPERTY()
    TMap<FISMBucketKey, UISMInstanceBucket*> BucketPools;
};