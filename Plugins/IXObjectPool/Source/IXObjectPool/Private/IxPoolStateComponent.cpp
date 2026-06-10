// Copyright maxi3777. All Rights Reserved.


#include "IxPoolStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "IxActivationInterface.h"

UIxPoolStateComponent::UIxPoolStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// 必须开启组件复制
	SetIsReplicatedByDefault(true);
	bIsPoolActive = false;
}

void UIxPoolStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UIxPoolStateComponent, bIsPoolActive);
}

void UIxPoolStateComponent::SetPoolActive(bool bActive)
{
	if (GetOwner()->HasAuthority() && bIsPoolActive != bActive)
	{
		bIsPoolActive = bActive;
		// 服务端直接在本地执行一次，触发本地效果（如果Server本身也是玩家的话）
		OnRep_IsPoolActive(); 
	}
}

void UIxPoolStateComponent::OnRep_IsPoolActive()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return;

	// 如果挂载此组件的 Actor 实现了我们的激活接口，则触发对应事件
	if (Owner->Implements<UIxActivationInterface>())
	{
		if (bIsPoolActive)
		{
			IIxActivationInterface::Execute_Activate(Owner);
		}
		else
		{
			IIxActivationInterface::Execute_Deactivate(Owner);
		}
	}
}
