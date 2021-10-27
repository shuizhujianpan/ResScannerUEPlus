// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlibOperationHelper.generated.h"

/**
 * 
 */
UCLASS()
class RESSCANNER_API UFlibOperationHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable,BlueprintPure)
	static TSubclassOf<class AGameModeBase> GetMapGameModeClassByAsset(UObject* World);
	UFUNCTION(BlueprintCallable,BlueprintPure)
	static TSubclassOf<class AGameModeBase> GetMapGameModeClass(UWorld* World);
};
