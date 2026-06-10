// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IxObjectPoolConfig.h"
#include "IxObjectPoolComponent.generated.h"

/**
 * 内部结构：管理每个类对应的数据集合
 */
USTRUCT()
struct FIxPoolData
{
	GENERATED_BODY()

	// 存放当前空闲的备用实例 (使用 TArray 方便快速 Pop)
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> FreeInstances;

	// 存放当前正在使用（非空闲）的实例 (使用 TSet 保证查找和移除为 O(1) 复杂度)
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> ActiveInstances;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IXOBJECTPOOL_API UIxObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UIxObjectPoolComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 指定的对象池配置数据资产
	UPROPERTY(EditAnywhere, Category = "Object Pool")
	TObjectPtr<UIxObjectPoolConfig> PoolConfig;

	/**
	 * 获取/取出对象
	 * @param ActorClass 要取出的类
	 * @param Transforms 目标位置/旋转/缩放数组，传入多少个 Transform 就生成/取出多少个实例
	 * @return 取出的对象数组
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (AutoCreateRefTerm = "Transforms"))
	TArray<AActor*> GetActors(TSubclassOf<AActor> ActorClass, const TArray<FTransform>& Transforms);

	/**
	 * 放回对象
	 * @param ActorClass 对象的类
	 * @param Actors 要放回的对象数组
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnActors(TSubclassOf<AActor> ActorClass, const TArray<AActor*>& Actors);

	
#if WITH_EDITORONLY_DATA
	// [编辑器工具] 目标DA保存路径 (例如 "/Game/DataAssets")
	UPROPERTY(EditAnywhere, Category = "Object Pool|Editor Tool")
	FString TargetDataAssetPath = TEXT("/Game");

	// [编辑器工具] 目标DA名称 (例如 "DA_MyObjectPoolConfig")
	UPROPERTY(EditAnywhere, Category = "Object Pool|Editor Tool")
	FString TargetDataAssetName = TEXT("DA_ObjectPoolConfig");
#endif

#if WITH_EDITOR
	/**
	 *[编辑器工具] 创建或修改 DataAsset，记录当前各个子类的峰值实例使用数量
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Object Pool|Editor Tool")
	void SavePeakUsageToDataAsset();
#endif

private:
	// 内部字典：以 UClass 作为 Key 集中管理所有对象的池数据
	UPROPERTY(Transient)
	TMap<UClass*, FIxPoolData> ObjectPools;

	// 内部函数：生成新实例
	AActor* SpawnNewInstance(TSubclassOf<AActor> ActorClass);

	// 内部函数：处理对象的激活与反激活状态切换
	static void SetActorActiveState(AActor* Actor, bool bActive);
};