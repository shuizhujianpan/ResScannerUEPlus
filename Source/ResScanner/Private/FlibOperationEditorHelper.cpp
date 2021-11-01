// Fill out your copyright notice in the Description page of Project Settings.


#include "FlibOperationEditorHelper.h"
#if WITH_EDITOR
	#include "BlueprintEditorSettings.h"
	#include "CompilerResultsLog.h"
	#include "KismetEditorUtilities.h"
#endif

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
		EBlueprintCompileOptions CompileOptions = EBlueprintCompileOptions::None;
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
