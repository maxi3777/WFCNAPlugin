// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WFCChainActor.generated.h"

class UWFCUnit;

UCLASS()
class WFCNA_API AWFCChainActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWFCChainActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WFCChain Settings")
	TSubclassOf<UWFCUnit> StartUnitClass;

	UPROPERTY()
	UWFCUnit* StartUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WFCChain Settings")
	bool bIsAutoRun;

	UFUNCTION(BlueprintCallable, Category="WFC Start")
	void WFCStart();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	double StartTime = 0;
};
