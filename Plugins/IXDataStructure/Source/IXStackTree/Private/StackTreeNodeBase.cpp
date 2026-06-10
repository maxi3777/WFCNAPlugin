// Fill out your copyright notice in the Description page of Project Settings.


#include "StackTreeNodeBase.h"
#include "StackTreeManagerBase.h"

UStackTreeNodeBase::UStackTreeNodeBase()
{
	bIsPrimaryNode = false;
	SelfManager = nullptr;
}

void UStackTreeNodeBase::Initialize()
{
	// 供子类重写：进行初始化
}

void UStackTreeNodeBase::Run()
{
	// 供子类重写：实现异步或执行逻辑
}

void UStackTreeNodeBase::Construct()
{
	// 供子类重写：实现自身数据的初始化逻辑
}

void UStackTreeNodeBase::Destruct()
{
	// 供子类重写：逆向操作 Construct (仅由 Manager 调用)
}

void UStackTreeNodeBase::AddChild(UStackTreeNodeBase* ChildNode)
{
	if (ChildNode)
	{
		// 强引用持有，防止子节点被 GC
		ChildrenNodes.Add(ChildNode);

		// 如果本节点为主节点，子节点归属自身的 SelfManager；否则跟随本节点归属于同一个 ParentManager
		ChildNode->ParentManager = bIsPrimaryNode ? SelfManager : ParentManager;
	}
}

void UStackTreeNodeBase::Preconstruct()
{
	if (bIsPrimaryNode)
	{
		// 主节点：将自己加到自身的 Manager 的自链中
		if (SelfManager)
		{
			SelfManager->AddSelfChainNodes(this);
		}
        
		// 如果有父级主节点，则将自己注册到父级主节点的 OtherPrimaryNodes 中
		if (ParentManager.IsValid())
		{
			ParentManager->AddOtherPrimaryNodes(this);
		}
	}
	else
	{
		// 非主节点：将自己加到父级主节点的自链中
		if (ParentManager.IsValid())
		{
			ParentManager->AddSelfChainNodes(this);
		}
	}

	// 调用自身构建
	Construct();

	// 构建结束后，递减相应 Manager 的计数器
	if (bIsPrimaryNode)
	{
		if (SelfManager)
		{
			SelfManager->ChangeConstructCount(-1);
		}
	}
	else
	{
		if (ParentManager.IsValid())
		{
			ParentManager->ChangeConstructCount(-1);
		}
	}
}

void UStackTreeNodeBase::ChainDestruct()
{
	// 仅限主节点可用
	if (bIsPrimaryNode && SelfManager)
	{
		// 触发所属责任区的一系列节点清理操作
		SelfManager->DestructNodes();
        
		// 内部节点已经全部 Destruct 完毕，此时清空强引用数组，让 UE 垃圾回收机制接管释放内存
		ChildrenNodes.Empty();
	}
}
