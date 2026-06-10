// Fill out your copyright notice in the Description page of Project Settings.

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
