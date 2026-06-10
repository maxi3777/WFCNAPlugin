// Copyright maxi3777. All Rights Reserved.


#include "StackTreeManagerBase.h"
#include "StackTreeNodeBase.h"

UStackTreeManagerBase::UStackTreeManagerBase()
{
	// 计数器初始值为 1
	ConstructCount = 1;
}

void UStackTreeManagerBase::AddSelfChainNodes(UStackTreeNodeBase* Node)
{
	if (Node)
	{
		// 压入栈底（后进）
		SelfChainNodes.Push(Node);
        
		// 获取子节点数量并增加计数器
		ChangeConstructCount(Node->ChildrenNodes.Num());
	}
}

void UStackTreeManagerBase::AddOtherPrimaryNodes(UStackTreeNodeBase* Node)
{
	if (Node)
	{
		// 压入栈底（后进）
		OtherPrimaryNodes.Push(Node);

		// 如果该节点（通常为主节点）拥有它自己的 Manager，监听它的结束委托
		if (Node->SelfManager)
		{
			// 使用 WeakLambda 绑定，防止本 Manager 已经销毁导致崩溃
			Node->SelfManager->OnConstructFinish.AddWeakLambda(this, [this]()
			{
				this->ChangeConstructCount(-1);
			});
		}
	}
}

void UStackTreeManagerBase::ChangeConstructCount(int32 Delta)
{
	ConstructCount += Delta;

	// 只有在行为是减小时才进行判断
	if (Delta < 0)
	{
		if (ConstructCount == 0)
		{
			OnConstructFinish.Broadcast();
		}
		else if (ConstructCount < 0)
		{
			OnChildChainRestartFinish.Broadcast();
		}
	}
}

void UStackTreeManagerBase::DestructNodes()
{
	// 1. 先逆序取出 OtherPrimaryNodes 中的节点（符合 LIFO: TArray::Pop() 天然从数组末尾取出并移除）
	while (OtherPrimaryNodes.Num() > 0)
	{
		if (UStackTreeNodeBase* Node = OtherPrimaryNodes.Pop())
		{
			Node->ChainDestruct();
		}
	}

	// 2. 再逆序取出 SelfChainNodes 中的节点（符合 LIFO）
	while (SelfChainNodes.Num() > 0)
	{
		if (UStackTreeNodeBase* Node = SelfChainNodes.Pop())
		{
			Node->Destruct();
		}
	}
}
