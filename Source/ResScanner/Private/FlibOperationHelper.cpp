// Fill out your copyright notice in the Description page of Project Settings.


#include "FlibOperationHelper.h"

TSubclassOf<class AGameModeBase> UFlibOperationHelper::GetMapGameModeClass(UWorld* World)
{
	TSubclassOf<class AGameModeBase> GameMode;
	if (World)
	{
		AWorldSettings*  Settings = World->GetWorldSettings();
		if (Settings)
		{
			GameMode = Settings->DefaultGameMode;
		}
	}
	return GameMode;
}

TSubclassOf<AGameModeBase> UFlibOperationHelper::GetMapGameModeClassByAsset(UObject* World)
{
	return UFlibOperationHelper::GetMapGameModeClass(Cast<UWorld>(World));
}
