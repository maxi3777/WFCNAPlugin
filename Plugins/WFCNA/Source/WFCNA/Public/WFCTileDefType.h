// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISMManagerTypes.h"
#include "WFCTileDefType.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct WFCNA_API FActorInfoEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;
};

USTRUCT(BlueprintType)
struct WFCNA_API FStaticMeshInfoEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FISMBucketKey BucketKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;
};
