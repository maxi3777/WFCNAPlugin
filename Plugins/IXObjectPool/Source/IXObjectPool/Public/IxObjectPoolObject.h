// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IxObjectPoolConfig.h"
#include "IxObjectPoolObject.generated.h"

/**
 * 内部结构：管理每个类对应的数据集合
 */
USTRUCT()
struct FIxActorPoolData
{
	GENERATED_BODY()

	// 存放当前空闲的备用实例
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> FreeInstances;

	// 存放当前正在使用（非空闲）的实例
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> ActiveInstances;
};

/**
 * 动态对象池 (基于 UObject)
 * 仅限服务端运行
 */
UCLASS(BlueprintType, Blueprintable)
class IXOBJECTPOOL_API UIxObjectPool : public UObject
{
	GENERATED_BODY()

public:
	// 必须重写此函数，以便蓝图节点和 SpawnActor 能正确获取到 World
	virtual UWorld* GetWorld() const override;

	/**
	 * 初始化对象池 (仅服务端有效)
	 * @param Config 对象池初始化配置数据
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Object Pool")
	void Initialize(UIxObjectPoolConfig* Config);

	/**
	 * 获取/取出对象 (仅服务端有效)
	 * @param ActorClass 要取出的类
	 * @param Transforms 目标位置/旋转/缩放数组
	 * @return 取出的对象数组
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Object Pool", meta = (AutoCreateRefTerm = "Transforms"))
	TArray<AActor*> GetActors(TSubclassOf<AActor> ActorClass, const TArray<FTransform>& Transforms);

	/**
	 * 放回对象 (仅服务端有效)
	 * @param ActorClass 对象的类
	 * @param Actors 要放回的对象数组
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Object Pool")
	void ReturnActors(TSubclassOf<AActor> ActorClass, const TArray<AActor*>& Actors);

	// UObject 生命周期结束时的回调（作为安全兜底）
	virtual void BeginDestroy() override;

private:
	// 内部字典：以 UClass 作为 Key 集中管理所有对象的池数据
	UPROPERTY(Transient)
	TMap<UClass*, FIxActorPoolData> ObjectPools;

	// 内部辅助函数：判断当前是否拥有网络权限（是否为服务端）
	bool HasAuthority() const;

	// 内部函数：生成新实例
	AActor* SpawnNewInstance(TSubclassOf<AActor> ActorClass);

	/**
	* 销毁池中所有的对象，并彻底清空对象池 (仅服务端有效)
	* 最佳实践：在不需要这个对象池时，手动调用此函数
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Object Pool")
	void ClearPool();

	// 内部函数：处理对象的激活与反激活状态切换
	static void SetActorActiveState(AActor* Actor, bool bActive);
};
