// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ISMManagerTypes.h"
#include "ISMInstanceBucket.h"
#include "ISMDirectorComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ISMMANAGER_API UISMDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UISMDirectorComponent();

    // 核心配置数据资产
    UPROPERTY(EditAnywhere, Category = "ISM Configuration")
    UISMManagerConfig* ConfigurationAsset;

    // 当某一种 Key 需要动态扩容时，每次新建的组件默认包含多少个实例
    UPROPERTY(EditAnywhere, Category = "ISM Configuration", meta=(ClampMin="10"))
    int32 ComponentMaxInstancesCount = 500;

#if WITH_EDITORONLY_DATA
    // 用于自动生成 DataAsset 时的保存路径
    UPROPERTY(EditAnywhere, Category = "ISM Editor Tools")
    FString DataAssetSavePath = TEXT("/Game/ISMConfig/DA_ISMManagerConfig");
#endif

public:
    virtual void InitializeComponent() override;

    // ==========================================
    // 公开运行时 API (仅服务器调用)
    // ==========================================
    UFUNCTION(BlueprintCallable, Category = "ISM Director")
    TArray<int32> BatchAllocateInstances(const FISMBucketKey& Key, const TArray<FTransform>& Transforms);

    UFUNCTION(BlueprintCallable, Category = "ISM Director")
    void BatchFreeInstances(const FISMBucketKey& Key, const TArray<int32>& Indices);

    // ==========================================
    // 编辑器工具
    // ==========================================
#if WITH_EDITOR
    UFUNCTION(CallInEditor, Category = "ISM Editor Tools")
    void GenerateOrUpdateDataAsset();
#endif

private:
    UISMInstanceBucket* GetOrCreateBucket(const FISMBucketKey& Key, int32 InitialInstancesCount = 0);

    // ==========================================
    // 网络同步 RPC (Multicast 同步给所有客户端)
    // ==========================================
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BatchAllocate(const FISMBucketKey& Key, const TArray<int32>& AllocatedIndices, const TArray<FTransform>& Transforms);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BatchFree(const FISMBucketKey& Key, const TArray<int32>& Indices);

private:
    UPROPERTY()
    TMap<FISMBucketKey, UISMInstanceBucket*> BucketPools;
};