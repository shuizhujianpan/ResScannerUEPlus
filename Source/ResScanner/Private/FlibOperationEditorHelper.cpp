// Fill out your copyright notice in the Description page of Project Settings.


#include "FlibOperationEditorHelper.h"
#if WITH_EDITOR
	#include "BlueprintEditorSettings.h"
	#include "Kismet2/CompilerResultsLog.h"
	#include "Kismet2/KismetEditorUtilities.h"
#endif

bool URule_CheckBlueprintError::Match_Implementation(UObject* Object, const FString& Asset)
{
	return UFlibOperationEditorHelper::BlueprintHasError(Object,false);
}

void UFlibOperationEditorHelper::CompileBlueprint(UObject* Blueprint, int32& OutNumError, int32& OutNumWarning)
{
#if WITH_EDITOR
	if(Blueprint)
	{
		UBlueprint* BlueprintIns = Cast<UBlueprint>(Blueprint);
		FCompilerResultsLog LogResults;
		LogResults.SetSourcePath(BlueprintIns->GetPathName());
		LogResults.BeginEvent(TEXT("Compile"));
		LogResults.bLogDetailedResults = GetDefault<UBlueprintEditorSettings>()->bShowDetailedCompileResults;
		LogResults.EventDisplayThresholdMs = GetDefault<UBlueprintEditorSettings>()->CompileEventDisplayThresholdMs;
		EBlueprintCompileOptions CompileOptions = EBlueprintCompileOptions::BatchCompile | EBlueprintCompileOptions::SkipSave;
		bool bSaveIntermediateBuildProducts = true;
		if( bSaveIntermediateBuildProducts )
		{
			CompileOptions |= EBlueprintCompileOptions::SaveIntermediateProducts;
		}
		FKismetEditorUtilities::CompileBlueprint(BlueprintIns, CompileOptions, &LogResults);

		LogResults.EndEvent();
		OutNumError = LogResults.NumErrors;
		OutNumWarning = LogResults.NumWarnings;
	}

#endif
}

bool UFlibOperationEditorHelper::BlueprintHasError(UObject* Blueprint,bool bWarningAsError)
{
	int32 OutNumError = 0;
	int32 OutNumWarning = 0;
	
	UFlibOperationEditorHelper::CompileBlueprint(Blueprint,OutNumError,OutNumWarning);

	bool bHasError = bWarningAsError ? (OutNumError > 0 || OutNumWarning > 0) : (OutNumError > 0);
	return bHasError;
}
