#include "ResScannerCommandlet.h"
#include "FMatchRuleTypes.h"
#include "FlibAssetParseHelper.h"
#include "TemplateHelper.hpp"
// engine header
#include <complex>

#include "CoreMinimal.h"

#include "ResScannerProxy.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/Paths.h"

#define COOKER_CONFIG_PARAM_NAME TEXT("-config=")

DEFINE_LOG_CATEGORY(LogResScannerCommandlet);
#define CONTENT_DIR TEXT("-contentdir=")
#define FILE_CHECK TEXT("-filecheck")
#define COMMIT_FILE_LIST TEXT("-filelist=")

#define GIT_CHECK TEXT("-gitcheck")
#define GIT_BEGIN_HASH TEXT("-gitbeginhash=")
#define GIT_END_HASH TEXT("-gitendhash=")

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
#include "FlibSourceControlHelper.h"

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
	
	bool bIsGitCheck = FParse::Param(FCommandLine::Get(), TEXT("gitcheck"));
	
	FString GitBeginHash,GitEndHash;
	if(bIsGitCheck)
	{
		bool bGitBeginHash = FParse::Value(*Params, *FString(GIT_BEGIN_HASH).ToLower(), GitBeginHash);
		bool bGitEndHash = FParse::Value(*Params, *FString(GIT_END_HASH).ToLower(), GitEndHash);
	
		if(bGitBeginHash && bGitEndHash)
		{
			TArray<FString> OutResault;
			if(UFlibSourceControlHelper::DiffVersionByGlobalGit(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()),GitBeginHash,GitEndHash,OutResault))
			{
				for(auto& File:OutResault)
				{
					FString Left,Right;
					File.Split(TEXT("."),&Left,&Right,ESearchCase::CaseSensitive,ESearchDir::FromEnd);
					FString FileName;
					{
						FString Path;
						Left.Split(TEXT("/"),&Path,&FileName,ESearchCase::CaseSensitive,ESearchDir::FromEnd);
					}
					
					FString AssetPath = FString::Printf(TEXT("/Game/%s.%s"),*Left,*FileName);
					FSoftObjectPath CurrentAsset(AssetPath);
					if(CurrentAsset.IsValid())
					{
						InAssets.Add(CurrentAsset);
					}
				}
			}
		}
	}
	
	FString JsonContent;
	if (FFileHelper::LoadFileToString(JsonContent, *config_path))
	{
		UE_LOG(LogResScannerCommandlet, Display, TEXT("%s"), *JsonContent);
		FScannerConfig ScannerConfig;
		TemplateHelper::TDeserializeJsonStringAsStruct(JsonContent,ScannerConfig);
		ScannerConfig.bByGlobalScanFilters = ScannerConfig.bByGlobalScanFilters || bIsFileCheck || bIsGitCheck;
		ScannerConfig.GlobalScanFilters.Assets.Append(InAssets);
		UResScannerProxy* ScannerProxy = NewObject<UResScannerProxy>();
		ScannerProxy->Init();
		ScannerProxy->SetScannerConfig(ScannerConfig);
		ScannerProxy->DoScan();
		const FMatchedResult& Result = ScannerProxy->GetScanResult();
		FString OutString;
		TemplateHelper::TSerializeStructAsJsonString(Result,OutString);
		UE_LOG(LogResScannerCommandlet, Display, TEXT("\nAsset Scan Result:\n%s"), *OutString);
	}
	if(FParse::Param(FCommandLine::Get(), TEXT("wait")))
	{
		system("pause");
	}
	return 0;
}