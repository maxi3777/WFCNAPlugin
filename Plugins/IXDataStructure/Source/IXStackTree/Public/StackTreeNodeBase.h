// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StackTreeNodeBase.generated.h"

class UStackTreeManagerBase;

UCLASS(BlueprintType, Blueprintable)
class IXSTACKTREE_API UStackTreeNodeBase : public UObject
{
	GENERATED_BODY()

public:
	UStackTreeNodeBase();

	// 判断是否为主节点
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StackTree")
	bool bIsPrimaryNode;

	// 若为主节点，拥有自身的 Manager，并使用 UPROPERTY 防止 GC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StackTree")
	UStackTreeManagerBase* SelfManager;

	// 每一个节点都持有一个弱引用指向其归属的主节点的 Manager
	TWeakObjectPtr<UStackTreeManagerBase> ParentManager;

	// 引用该节点的所有子节点，并防止其 GC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StackTree")
	TArray<UStackTreeNodeBase*> ChildrenNodes;

public:

	// 初始化
	virtual void Initialize();
	
	// 执行一些运行逻辑，通常包含异步操作
	virtual void Run();

	// 使用节点自身的数据进行操作
	virtual void Construct();

	// 对 Construct 所执行的操作进行逆操作
	// 说明：仅限 Manager 调用
	virtual void Destruct();

public:
	// 添加子节点并分配 ParentManager
	void AddChild(UStackTreeNodeBase* ChildNode);

	// 注册节点到 Manager 的责任链，处理计数器并调用 Construct
	void Preconstruct();

	// 仅限主节点可用，触发生命周期的销毁阶段，最后清空子树触发 GC
	void ChainDestruct();
};