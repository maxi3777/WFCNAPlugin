// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StackTreeManagerBase.generated.h"

class UStackTreeNodeBase;

// 声明多播委托
DECLARE_MULTICAST_DELEGATE(FOnStackTreeManagerConstructFinish);
DECLARE_MULTICAST_DELEGATE(FOnStackTreeManagerChildChainRestartFinish);

UCLASS(BlueprintType, Blueprintable)
class IXSTACKTREE_API UStackTreeManagerBase : public UObject
{
	GENERATED_BODY()

public:
	UStackTreeManagerBase();

	// 存储非主节点，作为 LIFO 栈使用
	UPROPERTY()
	TArray<UStackTreeNodeBase*> SelfChainNodes;

	// 存储子树的主节点，作为 LIFO 栈使用
	UPROPERTY()
	TArray<UStackTreeNodeBase*> OtherPrimaryNodes;

	// 计数器，初始值为 1
	int32 ConstructCount;

	// 委托
	FOnStackTreeManagerConstructFinish OnConstructFinish;
	FOnStackTreeManagerChildChainRestartFinish OnChildChainRestartFinish;

public:
	// 将节点添加到自身链表，并更新 ConstructCount
	void AddSelfChainNodes(UStackTreeNodeBase* Node);

	// 将其他主节点添加到其他主节点链表，并绑定监听以更新自身 ConstructCount
	void AddOtherPrimaryNodes(UStackTreeNodeBase* Node);

	// 修改 ConstructCount，并在减小时做相关逻辑广播
	void ChangeConstructCount(int32 Delta);

	// 逆序析构（Pop）所管理的节点，清理栈
	void DestructNodes();
};