// Fill out your copyright notice in the Description page of Project Settings.


#include "WFCModule.h"

DEFINE_LOG_CATEGORY(LogWFC);


#define LOCTEXT_NAMESPACE "FWFCModule"

void FWFCModule::StartupModule()
{
}

void FWFCModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWFCModule, WFCNA)
