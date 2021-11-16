#include "ResScannerCommandlet.h"
#include "FMatchRuleTypes.h"
#include "FlibAssetParseHelper.h"
#include "TemplateHelper.hpp"
// engine header
#include <complex>

#include "ReplacePropertyHelper.hpp"
#include "ResScannerProxy.h"

#include "CoreMinimal.h"
#include "AssetRegistryModule.h"
#include "Kismet/KismetStringLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/Paths.h"

#define ADD_GLOBAL_FILTER TEXT("globalScanFilters.filters")
#define ADD_GLOBAL_ASSETS TEXT("globalScanFilters.assets")
#define ADD_GLOBAL_IGNORE_FILTER TEXT("globalIgnoreFilters.filters")
#define ADD_GLOBAL_IGNORE_ASSETS TEXT("globalIgnoreFilters.assets")

#define COOKER_CONFIG_PARAM_NAME TEXT("-config=")

DEFINE_LOG_CATEGORY(LogResScannerCommandlet);
#define CONTENT_DIR TEXT("-contentdir=")
#define FILE_CHECK TEXT("-filecheck")
#define COMMIT_FILE_LIST TEXT("-filelist=")

TArray<FSoftObjectPath> GetCommitFileListObjects(const FString& ContentDir,const FString& FileList)
{
	FString NormalContentDir = ContentDir;
	FPaths::NormalizeFilename(NormalContentDir);
	
	TArray<FSoftObjectPath>	result;
	TArray<FString> FilesArray;
	FileList.ParseIntoArray(FilesArray,TEXT(","));
	for(auto& File:FilesArray)
	{
		FPaths::NormalizeFilename(File);
		if(File.StartsWith(NormalContentDir))
		{
			File.RemoveAt(0,NormalContentDir.Len());
			File = FString::Printf(TEXT("/Game/%s"),*File);
			FSoftObjectPath ObjectPath(File);

			if(ObjectPath.IsValid())
			{
				result.Emplace(ObjectPath);
			}
		}
	}
	return result;
}


int32 UResScannerCommandlet::Main(const FString& Params)
{
	UE_LOG(LogResScannerCommandlet, Display, TEXT("UResScannerCommandlet::Main"));
	FString config_path;
	bool bConfigStatus = FParse::Value(*Params, *FString(COOKER_CONFIG_PARAM_NAME).ToLower(), config_path);
	if (!bConfigStatus)
	{
		UE_LOG(LogResScannerCommandlet, Error, TEXT("not -config=xxxx.json params."));
		return -1;
	}
	if (!FPaths::FileExists(config_path))
	{
		UE_LOG(LogResScannerCommandlet, Error, TEXT("cofnig file %s not exists."), *config_path);
		return -1;
	}
	
	bool bIsFileCheck = FParse::Param(FCommandLine::Get(), TEXT("filecheck"));
	
	TArray<FSoftObjectPath> InAssets;
	FString CommitFileList;
	bool bFileListStatus = FParse::Value(*Params, *FString(COMMIT_FILE_LIST).ToLower(), CommitFileList);
	if (!bFileListStatus)
	{
		UE_LOG(LogResScannerCommandlet, Warning, TEXT("not %s=."),COMMIT_FILE_LIST);
	}
	else
	{
		InAssets = GetCommitFileListObjects(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()),CommitFileList);
	}


	if(IsRunningCommandlet())
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().SearchAllAssets(true);
	}

	TMap<FString, FString> TokenValues = ReplacePropertyHelper::GetCommandLineParamsMap(Params);
	
	
	FString JsonContent;
	int iProcessResult = 0;
	if (FFileHelper::LoadFileToString(JsonContent, *config_path))
	{
		UE_LOG(LogResScannerCommandlet, Display, TEXT("%s"), *JsonContent);
		FScannerConfig ScannerConfig;
		TemplateHelper::TDeserializeJsonStringAsStruct(JsonContent,ScannerConfig);
		ReplacePropertyHelper::ReplaceProperty(ScannerConfig,TokenValues);
		
		// replace RuleWhileListIDs
		for(const auto& Value:ReplacePropertyHelper::GetArrayElementByTokens(TEXT("RuleWhileListIDs"),TokenValues))
		{
			ScannerConfig.RuleWhileListIDs.Add(UKismetStringLibrary::Conv_StringToInt(Value));
		}
		ScannerConfig.GlobalScanFilters.Filters = ReplacePropertyHelper::ParserFilters(Params,ADD_GLOBAL_FILTER);
		ScannerConfig.GlobalScanFilters.Assets = ReplacePropertyHelper::ParserAssets(Params,ADD_GLOBAL_ASSETS);
		ScannerConfig.GlobalIgnoreFilters.Filters = ReplacePropertyHelper::ParserFilters(Params,ADD_GLOBAL_IGNORE_FILTER);
		ScannerConfig.GlobalIgnoreFilters.Assets = ReplacePropertyHelper::ParserAssets(Params,ADD_GLOBAL_IGNORE_ASSETS);
		
		ScannerConfig.bByGlobalScanFilters = ScannerConfig.bByGlobalScanFilters || bIsFileCheck;
		ScannerConfig.GlobalScanFilters.Assets.Append(InAssets);
		UResScannerProxy* ScannerProxy = NewObject<UResScannerProxy>();
		ScannerProxy->AddToRoot();
		ScannerProxy->SetScannerConfig(ScannerConfig);
		ScannerProxy->Init();
		ScannerProxy->DoScan();
		const FMatchedResult& Result = ScannerProxy->GetScanResult();
		FString OutString = ScannerProxy->SerializeResult();
		
		UE_LOG(LogResScannerCommandlet, Display, TEXT("\nAsset Scan Result:\n%s"), *OutString);
		if(Result.MatchedAssets.Num())
		{
			iProcessResult = -1;
		}
	}
	if(FParse::Param(FCommandLine::Get(), TEXT("wait")))
	{
		system("pause");
	}
	return iProcessResult;
}
