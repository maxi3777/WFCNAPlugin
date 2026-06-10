// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IxPoolStateComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class IXOBJECTPOOL_API UIxPoolStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UIxPoolStateComponent();

	// 提供给服务端的接口，用于修改池激活状态
	void SetPoolActive(bool bActive);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 同步的激活状态
	UPROPERTY(ReplicatedUsing = OnRep_IsPoolActive)
	bool bIsPoolActive;

	UFUNCTION()
	void OnRep_IsPoolActive();
};