// Fill out your copyright notice in the Description page of Project Settings.

#include "ISMInstanceBucket.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

void UISMInstanceBucket::InitializeBucket(AActor* InOwnerActor, const FISMBucketKey& InKey, int32 InInitialInstancesCount, int32 InComponentMaxInstancesCount)
{
    OwnerActor = InOwnerActor;
    BucketKey = InKey;
    ComponentMaxInstancesCount = InComponentMaxInstancesCount;

    if (InInitialInstancesCount > 0)
    {
        CreateAndAddNewComponent(InInitialInstancesCount);
    }
}

void UISMInstanceBucket::CreateAndAddNewComponent(int32 InstancesToCreate)
{
    if (!OwnerActor || InstancesToCreate <= 0) return;

    UInstancedStaticMeshComponent* NewComp;
    if (BucketKey.bUseHISM)
    {
        NewComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(OwnerActor);
    }
    else
    {
        NewComp = NewObject<UInstancedStaticMeshComponent>(OwnerActor);
    }

    NewComp->SetStaticMesh(BucketKey.StaticMesh);
    NewComp->SetMaterial(0, BucketKey.Material);
    if (BucketKey.CollisionProfile != NAME_None)
    {
        NewComp->SetCollisionProfileName(BucketKey.CollisionProfile);
    }
    else
    {
        NewComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    NewComp->RegisterComponent();
    OwnerActor->AddInstanceComponent(NewComp);

    int32 NewCompIndex = AllComponents.Add(NewComp);
    TotalInstanceCapacity += InstancesToCreate;

    NewComp->PreAllocateInstancesMemory(InstancesToCreate);

    // 初始化时临时关闭建树以加速
    UHierarchicalInstancedStaticMeshComponent* HISMComp = Cast<UHierarchicalInstancedStaticMeshComponent>(NewComp);
    if (HISMComp) HISMComp->bAutoRebuildTreeOnInstanceChanges = false;

    TArray<FTransform> ZeroTransforms;
    FTransform ZeroTransform;
    ZeroTransform.SetScale3D(FVector::ZeroVector);
    ZeroTransforms.Init(ZeroTransform, InstancesToCreate);

    NewComp->AddInstances(ZeroTransforms, false, false, false);

    // 恢复建树，底层会自动接管未来的异步更新
    if (HISMComp)
    {
        HISMComp->bAutoRebuildTreeOnInstanceChanges = true;
        HISMComp->BuildTreeIfOutdated(true, false); 
    }

    // 逆序压栈
    FreeIndices.Reserve(FreeIndices.Num() + InstancesToCreate);
    for (int32 LocalIdx = InstancesToCreate - 1; LocalIdx >= 0; --LocalIdx)
    {
        FreeIndices.Push(EncodeIndex(NewCompIndex, LocalIdx));
    }
}

TArray<int32> UISMInstanceBucket::BatchAllocateInstances(const TArray<FTransform>& Transforms)
{
    int32 NeededCount = Transforms.Num();
    TArray<int32> ResultIndices;
    if (NeededCount == 0) return ResultIndices;
    ResultIndices.Reserve(NeededCount);

    // 动态扩容
    if (FreeIndices.Num() < NeededCount)
    {
        int32 Shortage = NeededCount - FreeIndices.Num();
        int32 CompsToCreate = FMath::CeilToInt((float)Shortage / ComponentMaxInstancesCount);
        for (int32 i = 0; i < CompsToCreate; ++i)
        {
            CreateAndAddNewComponent(ComponentMaxInstancesCount);
        }
    }

    // 弹栈
    for (int32 i = 0; i < NeededCount; ++i)
    {
        ResultIndices.Add(FreeIndices.Pop(EAllowShrinking::No));
    }

    // 复用客户端的表现同步逻辑（核心的连续块合并拷贝）
    ClientApplyTransforms(ResultIndices, Transforms);

    return ResultIndices;
}

void UISMInstanceBucket::BatchFreeInstances(const TArray<int32>& Indices)
{
    if (Indices.IsEmpty()) return;

    TArray<int32> SortedIndices = Indices;
    SortedIndices.Sort(); // 升序排列保证连续性处理

    // 复用客户端的表现同步逻辑执行彻底隐藏（Scale=0）
    ClientFreeTransforms(SortedIndices);

    // 逆序压栈以完美恢复 LIFO 的顺向连续分配
    FreeIndices.Reserve(FreeIndices.Num() + SortedIndices.Num());
    for (int32 i = SortedIndices.Num() - 1; i >= 0; --i)
    {
        FreeIndices.Push(SortedIndices[i]);
    }
}

void UISMInstanceBucket::ClientApplyTransforms(const TArray<int32>& Indices, const TArray<FTransform>& Transforms)
{
    if (Indices.Num() != Transforms.Num()) return;

    // 如果客户端收到越界的索引，被动扩容，保证数组安全
    int32 MaxCompIdx = -1;
    for (int32 Idx : Indices)
    {
        MaxCompIdx = FMath::Max(MaxCompIdx, Idx >> INSTANCE_INDEX_BITS);
    }
    while (AllComponents.Num() <= MaxCompIdx)
    {
        CreateAndAddNewComponent(ComponentMaxInstancesCount);
    }

    // O(1) 连续块合并批处理
    int32 CurrentComp = -1;
    int32 CurrentLocalStart = -1;
    int32 CurrentCount = 0;
    int32 TransformStart = 0;

    TSet<UInstancedStaticMeshComponent*> ModifiedComps;

    auto FlushChunk = [&]() {
        if (CurrentCount > 0 && AllComponents.IsValidIndex(CurrentComp)) {
            UInstancedStaticMeshComponent* Comp = AllComponents[CurrentComp];
            TArrayView<const FTransform> TransformsView(Transforms.GetData() + TransformStart, CurrentCount);
            Comp->BatchUpdateInstancesTransforms(CurrentLocalStart, TransformsView, true, false, true); // 不立即 Dirty
            ModifiedComps.Add(Comp);
        }
    };

    for (int32 i = 0; i < Indices.Num(); ++i)
    {
        int32 CompIdx, LocalIdx;
        DecodeIndex(Indices[i], CompIdx, LocalIdx);

        if (CurrentCount == 0 || CompIdx != CurrentComp || LocalIdx != CurrentLocalStart + CurrentCount)
        {
            FlushChunk();
            CurrentComp = CompIdx;
            CurrentLocalStart = LocalIdx;
            CurrentCount = 1;
            TransformStart = i;
        }
        else
        {
            CurrentCount++;
        }
    }
    FlushChunk();

    for (UInstancedStaticMeshComponent* Comp : ModifiedComps)
    {
        Comp->MarkRenderStateDirty(); // 统一通知渲染器
    }
}

void UISMInstanceBucket::ClientFreeTransforms(const TArray<int32>& Indices)
{
    if (Indices.IsEmpty()) return;

    int32 CurrentComp = -1;
    int32 CurrentLocalStart = -1;
    int32 CurrentCount = 0;

    TSet<UInstancedStaticMeshComponent*> ModifiedComps;
    FTransform ZeroTransform;
    ZeroTransform.SetScale3D(FVector::ZeroVector);

    auto FlushChunk = [&]() {
        if (CurrentCount > 0 && AllComponents.IsValidIndex(CurrentComp)) {
            UInstancedStaticMeshComponent* Comp = AllComponents[CurrentComp];
            Comp->BatchUpdateInstancesTransform(CurrentLocalStart, CurrentCount, ZeroTransform, true, false, true);
            ModifiedComps.Add(Comp);
        }
    };

    for (int32 i = 0; i < Indices.Num(); ++i)
    {
        int32 CompIdx, LocalIdx;
        DecodeIndex(Indices[i], CompIdx, LocalIdx);

        if (CurrentCount == 0 || CompIdx != CurrentComp || LocalIdx != CurrentLocalStart + CurrentCount)
        {
            FlushChunk();
            CurrentComp = CompIdx;
            CurrentLocalStart = LocalIdx;
            CurrentCount = 1;
        }
        else
        {
            CurrentCount++;
        }
    }
    FlushChunk();

    for (UInstancedStaticMeshComponent* Comp : ModifiedComps)
    {
        Comp->MarkRenderStateDirty();
    }
}