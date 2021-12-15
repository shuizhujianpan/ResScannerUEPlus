// Fill out your copyright notice in the Description page of Project Settings.


#include "FlibAssetParseHelper.h"
#include "TemplateHelper.hpp"

// engine header
#include "Async/Async.h"
#include "Kismet/KismetStringLibrary.h"
#include "AssetRegistryModule.h"
#include "ARFilter.h"
#include "FlibSourceControlHelper.h"
#include "Engine/AssetManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

DEFINE_LOG_CATEGORY(LogFlibAssetParseHelper);

void UFlibAssetParseHelper::CreateSaveFileNotify(const FText& InMsg, const FString& InSavedFile,SNotificationItem::ECompletionState NotifyType)
{
	AsyncTask(ENamedThreads::GameThread,[InMsg,InSavedFile,NotifyType]()
	{
		auto Message = InMsg;
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 5.0f;
		Info.bUseSuccessFailIcons = false;
		Info.bUseLargeFont = false;

		const FString HyperLinkText = InSavedFile;
		Info.Hyperlink = FSimpleDelegate::CreateLambda(
			[](FString SourceFilePath)
			{
				FPlatformProcess::ExploreFolder(*SourceFilePath);
			},
			HyperLinkText
		);
		Info.HyperlinkText = FText::FromString(HyperLinkText);
		FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(NotifyType);
	});
}

FProperty* UFlibAssetParseHelper::GetPropertyByName(UObject* Obj, const FString& PropertyName)
{
	FProperty* Result = nullptr;
	for(TFieldIterator<FProperty> PropertyIter(Obj->GetClass());PropertyIter;++PropertyIter)
	{
		if(PropertyIter->GetName().Equals(PropertyName))
		{
			Result = *PropertyIter;
		}
		// UE_LOG(LogTemp,Log,TEXT("Property Name: %s"),*PropertyIter->GetName());
	}
	return Result;
}

FString UFlibAssetParseHelper::GetPropertyValueByName(UObject* Obj, const FString& PropertyName)
{
	FString Result;
	FProperty* Property = UFlibAssetParseHelper::GetPropertyByName(Obj, PropertyName);
	if(Property)
	{
		Property->ExportTextItem(Result,Property->ContainerPtrToValuePtr<void*>(Obj),TEXT(""),NULL,0);
	}
	return Result;
}

TArray<FAssetData> UFlibAssetParseHelper::GetAssetsByFiltersByClass(const TArray<UClass*>& AssetTypes, const TArray<FDirectoryPath>& FilterDirectorys, bool bRecursiveClasses)
{
	TArray<FString> Types;
	for(auto& Type:AssetTypes)
	{
		if(IsValid(Type))
		{
			Types.AddUnique(Type->GetName());
		}
	}
	return UFlibAssetParseHelper::GetAssetsByFilters(Types,FilterDirectorys,bRecursiveClasses);
}

TArray<FAssetData> UFlibAssetParseHelper::GetAssetsByFilters(const TArray<FString>& AssetTypes,
                                                             const TArray<FDirectoryPath>& FilterDirectorys, bool bRecursiveClasses)
{
	TArray<FString> FilterPaths;
	for(const auto& Directory:FilterDirectorys)
	{
		FilterPaths.AddUnique(Directory.Path);
	}
	return UFlibAssetParseHelper::GetAssetsByFilters(AssetTypes,FilterPaths,bRecursiveClasses);
}

TArray<FAssetData> UFlibAssetParseHelper::GetAssetsByFilters(const TArray<FString>& AssetTypes,
                                                             const TArray<FString>& FilterPaths, bool bRecursiveClasses)
{
	TArray<FAssetData> result;
	if(FilterPaths.Num())
	{
		FARFilter Filter;
		Filter.PackagePaths.Append(FilterPaths);
		Filter.ClassNames.Append(AssetTypes);
		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = bRecursiveClasses;
		UFlibAssetParseHelper::GetAssetRegistry().GetAssets(Filter, result);	
	}

	return result;
}

TArray<FAssetData> UFlibAssetParseHelper::GetAssetsByObjectPath(const TArray<FSoftObjectPath>& SoftObjectPaths)
{
	TArray<FAssetData> result;
	UAssetManager& AssetManager = UAssetManager::Get();
	for(const auto& ObjectPath:SoftObjectPaths)
	{
		FAssetData OutAssetData;
		if (AssetManager.GetAssetDataForPath(ObjectPath, OutAssetData) && OutAssetData.IsValid())
		{
			result.AddUnique(OutAssetData);
		}
	}
	return result;
}

TArray<FAssetData> UFlibAssetParseHelper::GetAssetsWithCachedByTypes(const TArray<FAssetData>& CachedAssets,
	const TArray<UClass*>& AssetTypes,bool bUseFilter,const TArray<FDirectoryPath>& FilterDirectorys,bool bRecursiveClasses)
{
	TArray<FString> Types;
	for(auto& Type:AssetTypes)
	{
		if(IsValid(Type))
		{
			Types.AddUnique(Type->GetName());
		}
	}
	if(!Types.Num())
	{
		UE_LOG(LogFlibAssetParseHelper,Error,TEXT("GetAssetsWithCachedByTypes Types is Emppty,Search all asset types."));
	}
	return UFlibAssetParseHelper::GetAssetsWithCachedByTypes(CachedAssets,Types,bUseFilter,FilterDirectorys,bRecursiveClasses);
}

TArray<FAssetData> UFlibAssetParseHelper::GetAssetsWithCachedByTypes(const TArray<FAssetData>& CachedAssets,
                                                                     const TArray<FString>& AssetTypes,bool bUseFilter,const TArray<FDirectoryPath>& FilterDirectorys,bool bRecursiveClasses)
{
	TArray<FAssetData> result;
	for(const auto& CachedAsset:CachedAssets)
	{
		for(const auto& Type:AssetTypes)
		{
			bool bCheckChild = true;
			if(bRecursiveClasses)
			{
				UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *Type, true);
				if(FoundClass)
				{
					bCheckChild = CachedAsset.GetClass()->IsChildOf(FoundClass);
				}
			}
			if(CachedAsset.AssetClass.ToString().Equals(Type) && bCheckChild)
			{
				if(bUseFilter)
				{
					for(const auto& Filter:FilterDirectorys)
					{
						if(CachedAsset.PackagePath.ToString().StartsWith(Filter.Path))
						{
							result.AddUnique(CachedAsset);
							break;
						}
					}
				}
				else
				{
					result.AddUnique(CachedAsset);
				}
				break;
			}
		}
	}
	return result;
}


IAssetRegistry& UFlibAssetParseHelper::GetAssetRegistry(bool bSearchAllAssets)
{
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if(bSearchAllAssets)
	{
		AssetRegistryModule.Get().SearchAllAssets(true);
	}
	return AssetRegistryModule.Get();
}

bool UFlibAssetParseHelper::IsIgnoreAsset(const FAssetData& AssetData, const TArray<FAssetFilters>& IgnoreRules)
{
	bool bIsIgnored = false;
	for(const auto& IgnoreRule:IgnoreRules)
	{
		for(const auto& Filter: IgnoreRule.Filters)
		{
			if(AssetData.PackagePath.ToString().StartsWith(Filter.Path))
			{
				bIsIgnored = true;
				break;
			}
		}
		if(!bIsIgnored)
		{
			for(const auto& Asset: IgnoreRule.Assets)
			{
				if(AssetData.ObjectPath.ToString().Equals(Asset.GetAssetPathString()))
				{
					bIsIgnored = true;
				}
			}
		}
		else
		{
			break;
		}
	}
	return bIsIgnored;
}

#define ENGINEDIR_MARK TEXT("[ENGINEDIR]")
#define ENGINE_CONTENT_DIR_MARK TEXT("[ENGINE_CONTENT_DIR]")
#define PROJECTDIR_MARK TEXT("[PROJECTDIR]")
#define PROJECT_CONTENT_DIR_MARK TEXT("[PROJECT_CONTENT_DIR]")
#define PROJECT_SAVED_DIR_MARK TEXT("[PROJECT_SAVED_DIR]")
#define PROJECT_CONFIG_DIR_MARK TEXT("[PROJECT_CONFIG_DIR]")

TMap<FString, FString> UFlibAssetParseHelper::GetReplacePathMarkMap()
{
	TMap<FString,FString> MarkMap;
	MarkMap.Add(ENGINEDIR_MARK,FPaths::EngineDir());
	MarkMap.Add(ENGINE_CONTENT_DIR_MARK,FPaths::EngineContentDir());
	MarkMap.Add(PROJECTDIR_MARK,FPaths::ProjectDir());
	MarkMap.Add(PROJECT_CONTENT_DIR_MARK,FPaths::ProjectContentDir());
	MarkMap.Add(PROJECT_SAVED_DIR_MARK,FPaths::ProjectSavedDir());
	MarkMap.Add(PROJECT_CONFIG_DIR_MARK,FPaths::ProjectConfigDir());
	return MarkMap;
}

FString UFlibAssetParseHelper::ReplaceMarkPath(const FString& Src)
{
	TMap<FString,FString> MarkMap = UFlibAssetParseHelper::GetReplacePathMarkMap();
	auto ReplaceProjectDir = [&MarkMap](const FString& OriginDir)->FString
	{
		TArray<FString> MarkKeys;
		MarkMap.GetKeys(MarkKeys);
		
		FString result = OriginDir;
		for(const auto& Key:MarkKeys)
		{
			if(OriginDir.StartsWith(Key))
			{
				result = OriginDir;
				result.RemoveFromStart(Key,ESearchCase::IgnoreCase);
				result = FPaths::Combine(MarkMap[Key],result);
				break;
			}
		}

		FPaths::MakeStandardFilename(result);
		result = FPaths::ConvertRelativePathToFull(result);
		return result;
	};
	return ReplaceProjectDir(Src);
}

TArray<FSoftObjectPath> ParserGitFilesToObjectPaths(TArray<FString> GitCommitFiles)
{
	TArray<FSoftObjectPath> ResultAssets;
	for(auto& File:GitCommitFiles)
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
			ResultAssets.Add(CurrentAsset);
		}
	}
	return ResultAssets;
}

TArray<FSoftObjectPath> UFlibAssetParseHelper::GetAssetsByGitChecker(const FGitChecker& GitChecker,
	const FString& GitBinaryOpt)
{
	TArray<FSoftObjectPath> ResultAssets;
	if(GitChecker.bGitCheck && GitChecker.bDiffCommit)
	{
		ResultAssets.Append(UFlibAssetParseHelper::GetAssetsByGitCommitHash(GitChecker.GetRepoDir(),GitChecker.BeginCommitHash,GitChecker.EndCommitHash,GitBinaryOpt));
	}
	if(GitChecker.bGitCheck && GitChecker.bUncommitFiles)
	{
		ResultAssets.Append(UFlibAssetParseHelper::GetAssetsByGitStatus(GitChecker.GetRepoDir(),GitBinaryOpt));
	}
	return ResultAssets;
}


TArray<FSoftObjectPath> UFlibAssetParseHelper::GetAssetsByGitStatus(const FString& RepoDir,
	const FString& GitBinaryOpt)
{
	auto IsUasset = [](const FString& File)->bool
	{
		TArray<FString> Extersions = {
			FPackageName::GetAssetPackageExtension(),
			FPackageName::GetMapPackageExtension()
		};
		bool bResult = false;
		for(const auto& Extersion:Extersions)
		{
			if(File.EndsWith(Extersion))
			{
				bResult = true;
				break;
			}
		}
		return bResult;
	};
	
	TArray<FSoftObjectPath> ResultAssets;
	TArray<FString> GitUnCommitFiles;
	if(UFlibSourceControlHelper::GitStatus(GitBinaryOpt,RepoDir,GitUnCommitFiles))
	{
		TArray<FString> Assets;
		for(const auto& UnCommitFile:GitUnCommitFiles)
		{
			if(IsUasset(UnCommitFile))
			{
				Assets.AddUnique(UnCommitFile);
			}
		}
		ResultAssets.Append(ParserGitFilesToObjectPaths(Assets));
	}
	return ResultAssets;
}

TArray<FSoftObjectPath> UFlibAssetParseHelper::GetAssetsByGitCommitHash(const FString& RepoDir,
	const FString& BeginHash, const FString& EndHand, const FString& GitBinaryOpt)
{
	TArray<FSoftObjectPath> ResultAssets;
	TArray<FString> GitCommitFiles;
	TArray<FString> OutErrorMessages;
	if(UFlibSourceControlHelper::DiffVersion(GitBinaryOpt,RepoDir,BeginHash,EndHand,GitCommitFiles,OutErrorMessages))
	{
		ResultAssets.Append(ParserGitFilesToObjectPaths(GitCommitFiles));
	}
	return ResultAssets;
}

void UFlibAssetParseHelper::CheckMatchedAssetsCommiter(FMatchedResult& MatchedResult, const FString& RepoDir)
{
	auto GetPackagePath = [](const FString& InPackageName)
	{
		FString AssetName;
		{
			int32 FoundIndex;
			InPackageName.FindLastChar('/', FoundIndex);
			if (FoundIndex != INDEX_NONE)
			{
				AssetName = UKismetStringLibrary::GetSubstring(InPackageName, FoundIndex + 1, InPackageName.Len() - FoundIndex);
			}
		}
		AssetName = InPackageName + TEXT(".") + AssetName;
		return AssetName;
	};
	for(auto& MatchedInfo:MatchedResult.MatchedAssets)
	{
		for(const auto& AssetPackageName:MatchedInfo.AssetPackageNames)
		{
			FString RealFile;
			FSoftObjectPath AssetObjectPaht = GetPackagePath(AssetPackageName);

			FString PackageExtension = FPackageName::GetAssetPackageExtension();
			{
				UPackage* Package = FindPackage(NULL, *AssetObjectPaht.GetAssetPathString());
				if (Package)
				{
					Package->FullyLoad();
				}
				else
				{
					Package = LoadPackage(NULL, *AssetObjectPaht.GetAssetPathString(), LOAD_None);
				}
				if(Package)
				{
					PackageExtension = Package->ContainsMap() ? FPackageName::GetMapPackageExtension() : FPackageName::GetAssetPackageExtension();
				}
			}
			
			FPackageName::TryConvertLongPackageNameToFilename(AssetPackageName,RealFile,*PackageExtension);
			
			RealFile = FPaths::ConvertRelativePathToFull(RealFile);
			if(!RealFile.IsEmpty())
			{
				RealFile.RemoveFromStart(RepoDir);
				FGitSourceControlRevisionData Data;
				FFileCommiter FileCommiter;
				if(UFlibSourceControlHelper::GetFileLastCommitByGlobalGit(RepoDir,RealFile,Data))
				{
					FileCommiter.File = AssetPackageName;
					FileCommiter.Commiter = Data.UserName;
				}
				else
				{
					FileCommiter.File = AssetPackageName;
				}
				MatchedInfo.AssetsCommiter.Add(FileCommiter);
			}
		}
	}
}

bool NameMatchOperator::Match(const FAssetData& AssetData,const FScannerMatchRule& Rule)
{
	bool bIsMatched = true;
	FString AssetName = AssetData.AssetName.ToString();
	for(const auto& MatchRule:Rule.NameMatchRules.Rules)
	{
		int32 OptionalMatchNum = 0;
		for(const auto& RuleItem:MatchRule.Rules)
		{
			bool bMatchResult = false;
			switch (MatchRule.MatchMode)
			{
			case ENameMatchMode::StartWith:
				{
					bMatchResult = AssetName.StartsWith(RuleItem.RuleText);
					break;
				}
			case ENameMatchMode::EndWith:
				{
					bMatchResult = AssetName.EndsWith(RuleItem.RuleText);
					break;
				}
			case ENameMatchMode::Wildcard:
				{
					bMatchResult = AssetName.MatchesWildcard(RuleItem.RuleText,ESearchCase::IgnoreCase);
					break;
				}
			}
			if(RuleItem.bReverseCheck)
			{
				bMatchResult = !bMatchResult;
			}
			
			if(bMatchResult)
			{
				OptionalMatchNum++;
			}
		}
		bool bIsMatchAllRules = (OptionalMatchNum == MatchRule.Rules.Num());
		// Optional中匹配成功的数量必须与配置的一致
		bIsMatched = (MatchRule.MatchLogic == EMatchLogic::Necessary) ? bIsMatchAllRules : (MatchRule.OptionalRuleMatchNum == OptionalMatchNum);
		if(!bIsMatched)
		{
			break;
		}
	}
	if(Rule.NameMatchRules.Rules.Num())
	{
		bIsMatched = Rule.NameMatchRules.bReverseCheck ? !bIsMatched : bIsMatched;
	}
	return bIsMatched;
}
bool PathMatchOperator::Match(const FAssetData& AssetData,const FScannerMatchRule& Rule)
{
	bool bIsMatched = true;
	FString AssetPath = AssetData.ObjectPath.ToString();
	for(const auto& MatchRule:Rule.PathMatchRules.Rules)
	{
		int32 OptionalMatchNum = 0;
		for(const auto& RuleItem:MatchRule.Rules)
		{
			bool bMatchResult = false;
			switch (MatchRule.MatchMode)
			{
			case EPathMatchMode::WithIn:
				{
					bMatchResult = AssetPath.StartsWith(RuleItem.RuleText);
					break;
				}
			case EPathMatchMode::Wildcard:
				{
					bMatchResult = AssetPath.MatchesWildcard(RuleItem.RuleText,ESearchCase::IgnoreCase);
					break;
				}
			}
			if(RuleItem.bReverseCheck)
			{
				bMatchResult = !bMatchResult;
			}
			if(bMatchResult)
			{
				OptionalMatchNum++;
			}
		}
		bool bIsMatchAllRules = (OptionalMatchNum == MatchRule.Rules.Num());
		bIsMatched = (MatchRule.MatchLogic == EMatchLogic::Necessary) ? bIsMatchAllRules : (MatchRule.OptionalRuleMatchNum == OptionalMatchNum);
		if(!bIsMatched)
		{
			break;
		}
	}
	if(Rule.PathMatchRules.Rules.Num())
	{
		bIsMatched = Rule.PathMatchRules.bReverseCheck ? !bIsMatched : bIsMatched;
	}
	return bIsMatched;
}

bool PropertyMatchOperator::Match(const FAssetData& AssetData,const FScannerMatchRule& Rule)
{
	bool bIsMatched = true;
	UObject* Asset = NULL;
	if(!!Rule.PropertyMatchRules.MatchRules.Num())
	{
		Asset  = AssetData.GetAsset();
	}

	auto IsFloatLambda = [](UObject* Object,const FString& PropertyName)->bool
	{
		FProperty* Property = UFlibAssetParseHelper::GetPropertyByName(Object, PropertyName);
		return Property->IsA(FFloatProperty::StaticClass());
	};
	auto IsFloatEqualLambda = [](const FString& L,const FString& R)->bool
	{
		float LValue = UKismetStringLibrary::Conv_StringToFloat(L);
		float RValue = UKismetStringLibrary::Conv_StringToFloat(R);
		return (LValue == RValue);
	};
	
	for(const auto& MatchRule:Rule.PropertyMatchRules.MatchRules)
	{
		int32 OptionalMatchNum = 0;
		for(const auto& PropertyRule:MatchRule.Rules)
		{
			bool bMatchResult = false;
			FString Value = UFlibAssetParseHelper::GetPropertyValueByName(Asset,PropertyRule.PropertyName);
			if(!Value.IsEmpty())
			{
				bool bIsFloatType = IsFloatLambda(Asset,PropertyRule.PropertyName);
				if(!bIsFloatType)
				{
					bMatchResult = Value.Equals(PropertyRule.MatchValue);
				}
				else
				{
					bMatchResult = IsFloatEqualLambda(Value,PropertyRule.MatchValue);
				}
				bMatchResult = (PropertyRule.MatchRule == EPropertyMatchRule::NotEqual) ? !bMatchResult : bMatchResult;
			}
			if(bMatchResult)
			{
				OptionalMatchNum++;
			}
		}
		bool bIsMatchAllRules = (OptionalMatchNum == MatchRule.Rules.Num());
		// Optional中匹配成功的数量必须与配置的一致
		bIsMatched = (MatchRule.MatchLogic == EMatchLogic::Necessary) ? bIsMatchAllRules : (MatchRule.OptionalRuleMatchNum == OptionalMatchNum);
		if(!bIsMatched)
		{
			break;
		}
	}
	if(Rule.PropertyMatchRules.MatchRules.Num())
	{
		bIsMatched = Rule.PropertyMatchRules.bReverseCheck ? !bIsMatched : bIsMatched;
	}
	return bIsMatched;
}

bool CustomMatchOperator::Match(const FAssetData& AssetData,const FScannerMatchRule& Rule)
{
	bool bIsMatched = true;
	for(auto ExOperator:Rule.CustomRules)
	{
		if(IsValid(ExOperator))
		{
			UOperatorBase* Operator = Cast<UOperatorBase>(ExOperator->GetDefaultObject());
			if(Operator)
			{
				bIsMatched = Operator->Match(AssetData.GetAsset(),AssetData.AssetClass.ToString());
				if(!bIsMatched && Operator->GetMatchLogic() == EMatchLogic::Necessary)
				{
					break;
				}
			}
			else
			{
				UE_LOG(LogFlibAssetParseHelper,Log,TEXT("% is Invalid UOperatorBase class!"),*ExOperator->GetName());
			}
		}
	}
	return bIsMatched;
}


