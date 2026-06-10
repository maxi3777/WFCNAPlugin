// Fill out your copyright notice in the Description page of Project Settings.


#include "IxObjectPoolComponent.h"
#include "Engine/World.h"
#include "IxActivationInterface.h"
#include "IxPoolStateComponent.h"

// 引入编辑器专用的头文件
#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#endif

UIxObjectPoolComponent::UIxObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	// 由于这个组件主要是纯服务端使用，可以不进行网络复制，只要它管理的 Actor 同步即可
	SetIsReplicatedByDefault(false); 
}

void UIxObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();

	// 仅服务端初始化对象池
	if (GetOwner()->HasAuthority() && PoolConfig)
	{
		for (const FIxObjectPoolConfigRow& Row : PoolConfig->PoolRows)
		{
			if (!Row.ActorClass || Row.InstancesCount <= 0) continue;

			FIxPoolData& PoolData = ObjectPools.FindOrAdd(Row.ActorClass);

			for (int32 i = 0; i < Row.InstancesCount; ++i)
			{
				if (AActor* NewActor = SpawnNewInstance(Row.ActorClass))
				{
					PoolData.FreeInstances.Add(NewActor);
				}
			}
		}
	}
}

TArray<AActor*> UIxObjectPoolComponent::GetActors(TSubclassOf<AActor> ActorClass, const TArray<FTransform>& Transforms)
{
	TArray<AActor*> ResultActors;

	// 安全校验，并且保证只有拥有权限的端（服务端）才能取对象
	if (!ActorClass || !GetOwner()->HasAuthority()) return ResultActors;

	FIxPoolData& PoolData = ObjectPools.FindOrAdd(ActorClass);

	for (const FTransform& Transform : Transforms)
	{
		AActor* ActorToUse = nullptr;

		// 1. 尝试从空闲池中获取
		while (PoolData.FreeInstances.Num() > 0)
		{
			ActorToUse = PoolData.FreeInstances.Pop();
			// 确保对象没有因为意外情况被 UE 的 GC 销毁
			if (IsValid(ActorToUse))
			{
				break;
			}
		}

		// 2. 如果空闲池不足（扩容）
		if (!IsValid(ActorToUse))
		{
			ActorToUse = SpawnNewInstance(ActorClass);
		}

		// 3. 激活逻辑与注册
		if (IsValid(ActorToUse))
		{
			ActorToUse->SetActorTransform(Transform, false, nullptr, ETeleportType::ResetPhysics);
			SetActorActiveState(ActorToUse, true);
			
			PoolData.ActiveInstances.Add(ActorToUse);
			ResultActors.Add(ActorToUse);
		}
	}

	return ResultActors;
}

void UIxObjectPoolComponent::ReturnActors(TSubclassOf<AActor> ActorClass, const TArray<AActor*>& Actors)
{
	if (!ActorClass || !GetOwner()->HasAuthority()) return;

	FIxPoolData* PoolData = ObjectPools.Find(ActorClass);
	if (!PoolData) return;

	for (AActor* Actor : Actors)
	{
		// 确保 Actor 确实属于当前的活跃对象列表中
		if (IsValid(Actor) && PoolData->ActiveInstances.Contains(Actor))
		{
			// 从活跃池移除并放回空闲池
			PoolData->ActiveInstances.Remove(Actor);
			PoolData->FreeInstances.Add(Actor);

			// 执行反激活逻辑
			SetActorActiveState(Actor, false);
		}
	}
}

AActor* UIxObjectPoolComponent::SpawnNewInstance(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass || !GetWorld()) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true;

	// 初始生成在原点，稍后会被移动
	AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (NewActor)
	{
		// 刚刚生成的实例默认进入“未激活”状态
		SetActorActiveState(NewActor, false);
	}
	return NewActor;
}

void UIxObjectPoolComponent::SetActorActiveState(AActor* Actor, bool bActive)
{
	if (!IsValid(Actor)) return;

	// 需求2：隐藏状态与碰撞切换
	Actor->SetActorHiddenInGame(!bActive);
	Actor->SetActorEnableCollision(bActive);

	// 如果 Actor 身上挂载了我们的网络同步组件，交给组件处理接口调用（解决网络同步痛点）
	if (UIxPoolStateComponent* StateComp = Actor->FindComponentByClass<UIxPoolStateComponent>())
	{
		StateComp->SetPoolActive(bActive);
	}
	else
	{
		// 如果没有状态组件，说明此 Actor 可能不需要网络多播表现，直接在当前端调用接口即可
		if (Actor->Implements<UIxActivationInterface>())
		{
			if (bActive)
			{
				IIxActivationInterface::Execute_Activate(Actor);
			}
			else
			{
				IIxActivationInterface::Execute_Deactivate(Actor);
			}
		}
	}
}

#if WITH_EDITOR
void UIxObjectPoolComponent::SavePeakUsageToDataAsset()
{
	if (TargetDataAssetPath.IsEmpty() || TargetDataAssetName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("SavePeakUsageToDataAsset Failed: Invalid Path or Name"));
		return;
	}

	// 1. 构建包路径
	FString PackageName = TargetDataAssetPath;
	if (!PackageName.EndsWith(TEXT("/")))
	{
		PackageName += TEXT("/");
	}
	PackageName += TargetDataAssetName;

	UPackage* Package = CreatePackage(*PackageName);
	if (!Package) return;
	Package->FullyLoad();

	// 2. 尝试加载或新建 DataAsset
	UIxObjectPoolConfig* DA = LoadObject<UIxObjectPoolConfig>(Package, *TargetDataAssetName, nullptr, LOAD_None, nullptr);
	bool bIsNewDA = false;

	if (!DA)
	{
		DA = NewObject<UIxObjectPoolConfig>(Package, UIxObjectPoolConfig::StaticClass(), *TargetDataAssetName, RF_Public | RF_Standalone);
		bIsNewDA = true;
	}

	bool bModified = false;

	// 3. 遍历当前池，计算活跃数量
	for (const auto& KVP : ObjectPools)
	{
		UClass* ActorClass = KVP.Key;
		int32 ActiveCount = KVP.Value.ActiveInstances.Num(); // 非空闲（正处于活跃状态）的数量

		// 查找 DA 里是否已有这个子类
		FIxObjectPoolConfigRow* FoundRow = DA->PoolRows.FindByPredicate([ActorClass](const FIxObjectPoolConfigRow& Row){
			return Row.ActorClass == ActorClass;
		});

		if (FoundRow)
		{
			// 若有，且当前实际活跃数量大于记录的，更新它
			if (ActiveCount > FoundRow->InstancesCount)
			{
				FoundRow->InstancesCount = ActiveCount;
				bModified = true;
			}
		}
		else
		{
			// 若 DA 中找不到对应的类，直接创建新的一行
			FIxObjectPoolConfigRow NewRow;
			NewRow.ActorClass = ActorClass;
			NewRow.InstancesCount = ActiveCount;
			DA->PoolRows.Add(NewRow);
			bModified = true;
		}
	}

	// 4. 执行保存操作
	if (bIsNewDA || bModified)
	{
		DA->MarkPackageDirty();

		if (bIsNewDA)
		{
			FAssetRegistryModule::AssetCreated(DA); // 通知编辑器资源浏览器创建了新资源
		}

		// 获取磁盘物理路径
		FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		
		// 固化到磁盘

		if (UPackage::SavePackage(Package, DA, *PackageFileName, SaveArgs))
		{
			UE_LOG(LogTemp, Log, TEXT("ObjectPool Config Saved Successfully to: %s"), *PackageFileName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save ObjectPool Config to: %s"), *PackageFileName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No updates needed for DataAsset (Peak values are lower or equal)."));
	}
}
#endif

