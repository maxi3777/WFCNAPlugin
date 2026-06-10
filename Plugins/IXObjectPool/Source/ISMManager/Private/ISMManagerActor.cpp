// Copyright maxi3777. All Rights Reserved.

#include "ISMManagerActor.h"
#include "Net/UnrealNetwork.h"

AISMManagerActor::AISMManagerActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // 开启网络同步，且保证该 Actor 始终对客户端可见 (避免被网络相关距离剔除)
    bReplicates = true;
    bAlwaysRelevant = true; 

    // 设置一个空场景根节点以容纳后续生成的 ISM 组件
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AISMManagerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AISMManagerActor, ComponentMaxInstancesCount);
    DOREPLIFETIME(AISMManagerActor, BucketInitData);
}

void AISMManagerActor::Initialize(UISMManagerConfig* Config, int32 MaxInstancesCount)
{
    // 仅允许服务器执行初始化
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("AISMManagerActor::Initialize MUST be called on the Server!"));
        return;
    }

    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("AISMManagerActor::Initialize received a null Config!"));
        return;
    }

    ComponentMaxInstancesCount = MaxInstancesCount;

    // 遍历 DataAsset，预先创建底层的 Bucket，并填充同步数据
    for (const FISMConfigRow& Row : Config->ConfigRows)
    {
        // 1. 本地 (服务器) 立即创建 Bucket
        GetOrCreateBucket(Row.BucketKey, Row.InstancesCount);

        // 2. 将数据推入同步数组
        FISMInitData InitData;
        InitData.Key = Row.BucketKey;
        InitData.InitialCount = Row.InstancesCount;
        BucketInitData.Add(InitData);
    }

    // 强制触发一次同步，确保刚连接的客户端立刻收到
    ForceNetUpdate();
}

void AISMManagerActor::OnRep_BucketInitData()
{
    // 客户端收到服务器下发的初始容量后，创建与服务器规模完全一模一样的初始组件
    // 这保证了服务器发来的底层索引（LocalIdx）在客户端绝不会越界
    for (const FISMInitData& Data : BucketInitData)
    {
        GetOrCreateBucket(Data.Key, Data.InitialCount);
    }
}

UISMInstanceBucket* AISMManagerActor::GetOrCreateBucket(const FISMBucketKey& Key, int32 InitialInstancesCount)
{
    if (UISMInstanceBucket** FoundBucket = BucketPools.Find(Key))
    {
        return *FoundBucket;
    }

    UISMInstanceBucket* NewBucket = NewObject<UISMInstanceBucket>(this);
    
    // 初始化 Bucket，由于移除了防抖延时，这里去掉了对应的参数
    NewBucket->InitializeBucket(this, Key, InitialInstancesCount, ComponentMaxInstancesCount);
    
    BucketPools.Add(Key, NewBucket);
    return NewBucket;
}

TArray<int32> AISMManagerActor::BatchAllocateInstances(const FISMBucketKey& Key, const TArray<FTransform>& Transforms)
{
    if (Transforms.IsEmpty()) return TArray<int32>();

    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("BatchAllocateInstances must be called on Server!"));
        return TArray<int32>();
    }

    UISMInstanceBucket* Bucket = GetOrCreateBucket(Key);
    TArray<int32> AllocatedIndices = Bucket->BatchAllocateInstances(Transforms);

    // 如果不是单机模式，广播给客户端应用视觉表现
    if (GetNetMode() != NM_Standalone)
    {
        Multicast_BatchAllocate(Key, AllocatedIndices, Transforms);
    }

    return AllocatedIndices;
}

void AISMManagerActor::BatchFreeInstances(const FISMBucketKey& Key, const TArray<int32>& Indices)
{
    if (Indices.IsEmpty()) return;

    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("BatchFreeInstances must be called on Server!"));
        return;
    }

    UISMInstanceBucket* Bucket = GetOrCreateBucket(Key);
    Bucket->BatchFreeInstances(Indices);

    // 同步给客户端执行 Scale 清零
    if (GetNetMode() != NM_Standalone)
    {
        Multicast_BatchFree(Key, Indices);
    }
}

void AISMManagerActor::Multicast_BatchAllocate_Implementation(const FISMBucketKey& Key, const TArray<int32>& AllocatedIndices, const TArray<FTransform>& Transforms)
{
    // 拦截服务器自己重复应用（因为主函数中已经应用过了）
    if (HasAuthority() && GetNetMode() != NM_Client) return;

    // 客户端如果发现未知的 Key（例如游戏运行中途生成的全新 Key），会自动被动创建一个容量为 0 的桶
    // Bucket 内部逻辑发现 LocalIdx 越界时，会自动用 ComponentMaxInstancesCount 进行扩容！
    UISMInstanceBucket* Bucket = GetOrCreateBucket(Key, 0);
    Bucket->ClientApplyTransforms(AllocatedIndices, Transforms);
}

void AISMManagerActor::Multicast_BatchFree_Implementation(const FISMBucketKey& Key, const TArray<int32>& Indices)
{
    if (HasAuthority() && GetNetMode() != NM_Client) return;

    if (UISMInstanceBucket** BucketPtr = BucketPools.Find(Key))
    {
        TArray<int32> SortedIndices = Indices;
        SortedIndices.Sort(); // 客户端同样执行排序，确保存储表现连续处理，降低 GPU 交互次数
        (*BucketPtr)->ClientFreeTransforms(SortedIndices);
    }
}