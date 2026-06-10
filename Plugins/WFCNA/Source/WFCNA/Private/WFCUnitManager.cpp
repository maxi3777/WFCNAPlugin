// Fill out your copyright notice in the Description page of Project Settings.


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
