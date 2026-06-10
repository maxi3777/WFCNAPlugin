// Fill out your copyright notice in the Description page of Project Settings.


#include "IxObjectPoolObject.h"
#include "Engine/World.h"
#include "IxActivationInterface.h"
#include "IxPoolStateComponent.h"

UWorld* UIxObjectPool::GetWorld() const
{
	// 防止在 CDO (类默认对象) 调用时报错
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	// 尝试通过 Outer 获取 World (通常实例化时 Outer 传入的是持有它的 Actor 或 World)
	if (UObject* Outer = GetOuter())
	{
		return Outer->GetWorld();
	}

	return nullptr;
}

bool UIxObjectPool::HasAuthority() const
{
	if (UWorld* World = GetWorld())
	{
		// 只要不是纯客户端，就认为是服务端（ListenServer 也是 Server）
		return World->GetNetMode() != NM_Client;
	}
	return false;
}

void UIxObjectPool::Initialize(UIxObjectPoolConfig* Config)
{
	// 权限检查与参数检查
	if (!HasAuthority() || !Config)
	{
		return;
	}

	for (const FIxObjectPoolConfigRow& Row : Config->PoolRows)
	{
		if (!Row.ActorClass || Row.InstancesCount <= 0) continue;

		FIxActorPoolData& PoolData = ObjectPools.FindOrAdd(Row.ActorClass);

		for (int32 i = 0; i < Row.InstancesCount; ++i)
		{
			if (AActor* NewActor = SpawnNewInstance(Row.ActorClass))
			{
				PoolData.FreeInstances.Add(NewActor);
			}
		}
	}
}

TArray<AActor*> UIxObjectPool::GetActors(TSubclassOf<AActor> ActorClass, const TArray<FTransform>& Transforms)
{
	TArray<AActor*> ResultActors;

	// 安全校验与权限校验
	if (!ActorClass || !HasAuthority()) return ResultActors;

	FIxActorPoolData& PoolData = ObjectPools.FindOrAdd(ActorClass);

	for (const FTransform& Transform : Transforms)
	{
		AActor* ActorToUse = nullptr;

		// 1. 尝试从空闲池中获取
		while (PoolData.FreeInstances.Num() > 0)
		{
			ActorToUse = PoolData.FreeInstances.Pop();
			// 确保对象有效且未被销毁
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

void UIxObjectPool::ReturnActors(TSubclassOf<AActor> ActorClass, const TArray<AActor*>& Actors)
{
	if (!ActorClass || !HasAuthority()) return;

	FIxActorPoolData* PoolData = ObjectPools.Find(ActorClass);
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

void UIxObjectPool::BeginDestroy()
{
	// 当这个 UObject 被垃圾回收时，触发一次安全兜底清理
	ClearPool();
	
	Super::BeginDestroy();
}

AActor* UIxObjectPool::SpawnNewInstance(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass || !GetWorld()) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true;

	// 初始生成在原点
	AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (NewActor)
	{
		// 刚刚生成的实例默认进入“未激活”状态
		SetActorActiveState(NewActor, false);
	}
	return NewActor;
}

void UIxObjectPool::ClearPool()
{
	UWorld* World = GetWorld();
	// 安全校验：如果关卡本身正在被销毁（比如切换地图），引擎底层会自动销毁所有 Actor。
	// 此时强行调用 Destroy() 可能会引发崩溃或警告，因此直接跳过。
	if (!World || World->bIsTearingDown)
	{
		ObjectPools.Empty();
		return;
	}

	// 只有服务端有权限销毁网络同步的 Actor
	if (HasAuthority())
	{
		for (auto& KVP : ObjectPools)
		{
			FIxActorPoolData& PoolData = KVP.Value;

			// 销毁所有空闲实例
			for (AActor* Actor : PoolData.FreeInstances)
			{
				if (IsValid(Actor))
				{
					Actor->Destroy();
				}
			}
			
			// 销毁所有正在使用的实例
			for (AActor* Actor : PoolData.ActiveInstances)
			{
				if (IsValid(Actor))
				{
					Actor->Destroy();
				}
			}
		}
	}
	
	// 清空映射表
	ObjectPools.Empty();
}

void UIxObjectPool::SetActorActiveState(AActor* Actor, bool bActive)
{
	if (!IsValid(Actor)) return;

	// 需求2：隐藏状态与碰撞切换
	Actor->SetActorHiddenInGame(!bActive);
	Actor->SetActorEnableCollision(bActive);

	// 网络状态同步处理
	if (UIxPoolStateComponent* StateComp = Actor->FindComponentByClass<UIxPoolStateComponent>())
	{
		StateComp->SetPoolActive(bActive);
	}
	else
	{
		// 无状态同步组件时的本地调用
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
