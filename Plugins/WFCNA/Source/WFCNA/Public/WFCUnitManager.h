// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StackTreeManagerBase.h"
#include "WFCUnitManager.generated.h"

struct FISMBucketKey;
class AISMManagerActor;
class UIxObjectPool;
class AISMHelperActor;
/**
 * 
 */
UCLASS()
class WFCNA_API UWFCUnitManager : public UStackTreeManagerBase
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TObjectPtr<UIxObjectPool> ObjectPool;

	UPROPERTY()
	TObjectPtr<AISMManagerActor> ISMManagerActor;

	virtual void BeginDestroy() override;
};
