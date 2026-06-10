// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IxActivationInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UIxActivationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 对象池激活/反激活接口
 */
class IXOBJECTPOOL_API IIxActivationInterface
{
	GENERATED_BODY()

public:
	// 从对象池取出时调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void Activate();

	// 放回对象池时调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void Deactivate();
};