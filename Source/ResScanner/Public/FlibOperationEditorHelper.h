// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlibOperationEditorHelper.generated.h"

/**
 * 
 */
UCLASS()
class RESSCANNER_API UFlibOperationEditorHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,meta=(AutoCreateRefTerm="OutNumError,OutNumWarning"))
	static void CompileBlueprint(UObject* Blueprint,int32& OutNumError,int32& OutNumWarning);
	
};
