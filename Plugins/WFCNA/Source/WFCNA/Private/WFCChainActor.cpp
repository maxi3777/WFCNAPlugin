// Copyright maxi3777. All Rights Reserved.


#include "WFCChainActor.h"

#include "StackTreeManagerBase.h"
#include "WFCUnit.h"
#include "HAL/PlatformTime.h" 
#include "Misc/OutputDevice.h"

// Sets default values
AWFCChainActor::AWFCChainActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AWFCChainActor::WFCStart()
{
    if(HasAuthority())
    {
		StartUnit->ChainDestruct();
		StartTime = FPlatformTime::Seconds();
		StartUnit->SelfManager->OnConstructFinish.AddLambda( [this, StartTime_ = StartTime]
		{
			double EndTime = FPlatformTime::Seconds();
			double ElapsedMS = (EndTime - StartTime_) * 1000.0;
			UE_LOG(LogTemp, Warning, TEXT("耗时 %.3f 毫秒"), ElapsedMS);
		});
		StartUnit->Run();
	}
}

// Called when the game starts or when spawned
void AWFCChainActor::BeginPlay()
{
	Super::BeginPlay();

    if(HasAuthority())
    {
    	StartUnit = NewObject<UWFCUnit>(this, StartUnitClass);
		StartUnit->SetGenRootTransform(GetActorTransform());
		StartUnit->Initialize();
		if (bIsAutoRun)
		{
			StartUnit->Run();
		}
    }
}


