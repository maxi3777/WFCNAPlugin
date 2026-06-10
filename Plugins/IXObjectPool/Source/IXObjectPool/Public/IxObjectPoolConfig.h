// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IxObjectPoolConfig.generated.h"

// 对象池每一行的配置
USTRUCT(BlueprintType)
struct FIxObjectPoolConfigRow
{
	GENERATED_BODY()

	// 要生成的Actor子类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	TSubclassOf<AActor> ActorClass;

	// 生成实例的数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool", meta = (ClampMin = "0"))
	int32 InstancesCount = 0;
};

/**
 * 对象池配置数据资产
 */
UCLASS(BlueprintType)
class IXOBJECTPOOL_API UIxObjectPoolConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// 配置表：每个类的默认生成数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	TArray<FIxObjectPoolConfigRow> PoolRows;
};
