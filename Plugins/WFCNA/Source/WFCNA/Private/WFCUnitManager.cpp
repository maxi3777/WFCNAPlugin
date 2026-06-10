// Copyright maxi3777. All Rights Reserved.


#include "WFCUnitManager.h"

#include "ISMManager/Public/ISMManagerActor.h"

void UWFCUnitManager::BeginDestroy()
{
	if (UWorld* World = GetWorld(); World && !World->bIsTearingDown)
	{
		if(ISMManagerActor){ISMManagerActor->Destroy();}
	}
	
	Super::BeginDestroy();
}
