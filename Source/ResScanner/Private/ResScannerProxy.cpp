#include "ResScannerProxy.h"

#include "FlibSourceControlHelper.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogResScannerProxy);
#define LOCTEXT_NAMESPACE "UResScannerProxy"

void UResScannerProxy::Init()
{
	MatchedResult.MatchedAssets.Empty();
	if(!GetScannerConfig().IsValid())
	{
		ScannerConfig = MakeShareable(new FScannerConfig);
	}
	MatchOperators.Add(TEXT("NameMatchRule"),MakeShareable(new NameMatchOperator));
	MatchOperators.Add(TEXT("PathMatchRule"),MakeShareable(new PathMatchOperator));
	MatchOperators.Add(TEXT("PropertyMatchRule"),MakeShareable(new PropertyMatchOperator));
	MatchOperators.Add(TEXT("CustomMatchRule"),MakeShareable(new CustomMatchOperator));
}

void UResScannerProxy::Shutdown()
{
	MatchedResult.MatchedAssets.Empty();
}

void UResScannerProxy::ScanSingleRule(const TArray<FAssetData>& GlobalAssets,const FScannerMatchRule& ScannerRule,int32 RuleID/* = 0*/)
{
	FRuleMatchedInfo RuleMatchedInfo;
	// FScannerMatchRule& ScannerRule = GetScannerConfig()->ScannerRules[RuleID];
	if(!ScannerRule.bEnableRule)
	{
		UE_LOG(LogResScannerProxy,Warning,TEXT("rule %s is missed!"),*ScannerRule.RuleName);
		return;
	}
	if(!ScannerRule.ScanFilters.Num() && !ScannerConfig->bByGlobalScanFilters)
	{
		UE_LOG(LogResScannerProxy,Warning,TEXT("rule %s not contain any filters!"),*ScannerRule.RuleName);
		return;
	}
	if(!ScannerRule.HasValidRules())
	{
		UE_LOG(LogResScannerProxy,Warning,TEXT("rule %s not contain any rules!"),*ScannerRule.RuleName);
		return;
	}
	UE_LOG(LogResScannerProxy,Display,TEXT("RuleName %s is Scanning."),*ScannerRule.RuleName);
	FString RuleConfig;
	TemplateHelper::TSerializeStructAsJsonString(ScannerRule,RuleConfig);
	UE_LOG(LogResScannerProxy,Display,TEXT("RuleName %s is Scanning. config:\n%s"),*ScannerRule.RuleName,*RuleConfig);
	
	TArray<FAssetData> FilterAssets;
	if(GetScannerConfig()->bByGlobalScanFilters || GetScannerConfig()->GitChecker.bGitCheck)
	{
		FilterAssets = UFlibAssetParseHelper::GetAssetsWithCachedByTypes(GlobalAssets,TArray<UClass*>{ScannerRule.ScanAssetType},ScannerRule.bGlobalAssetMustMatchFilter,ScannerRule.ScanFilters,ScannerRule.RecursiveClasses);
	}
	if(!GetScannerConfig()->bBlockRuleFilter)
	{
		FilterAssets.Append(UFlibAssetParseHelper::GetAssetsByFiltersByClass(TArray<UClass*>{ScannerRule.ScanAssetType},ScannerRule.ScanFilters,ScannerRule.RecursiveClasses));
	}
	RuleMatchedInfo.RuleName = ScannerRule.RuleName;
	RuleMatchedInfo.RuleDescribe = ScannerRule.RuleDescribe;
	RuleMatchedInfo.RuleID  = RuleID;
	TArray<FAssetFilters> FinalIgnoreFilters;
	FinalIgnoreFilters.Add(GetScannerConfig()->GlobalIgnoreFilters);
	
	FinalIgnoreFilters.Add(ScannerRule.IgnoreFilters);
	for(const auto& Asset:FilterAssets)
	{
		if(!UFlibAssetParseHelper::IsIgnoreAsset(Asset,FinalIgnoreFilters))
		{
			bool bMatchAllRules = true;
			for(const auto& Operator:GetMatchOperators())
			{
				bMatchAllRules = Operator.Value->Match(Asset,ScannerRule);
				if(!bMatchAllRules)
				{
					break;
				}
			}
			if(bMatchAllRules)
			{
				RuleMatchedInfo.Assets.AddUnique(Asset);
				RuleMatchedInfo.AssetPackageNames.AddUnique(Asset.PackageName.ToString());
			}
		}
	}
	if(!!RuleMatchedInfo.Assets.Num())
	{
		if(ScannerRule.bEnablePostProcessor)
		{
			// 对扫描之后的资源进行后处理（可以执行自动化处理操作）
			PostProcessorMatchRule(ScannerRule,RuleMatchedInfo);
		}
		MatchedResult.MatchedAssets.Add(RuleMatchedInfo);
	}
}

void UResScannerProxy::DoScan()
{
	MatchedResult.MatchedAssets.Empty();
	TArray<FAssetData> GlobalAssets;
	if(GetScannerConfig()->bByGlobalScanFilters)
	{
		 GlobalAssets = UFlibAssetParseHelper::GetAssetsByObjectPath(GetScannerConfig()->GlobalScanFilters.Assets);
		 GlobalAssets.Append(UFlibAssetParseHelper::GetAssetsByFiltersByClass(TArray<UClass*>{},GetScannerConfig()->GlobalScanFilters.Filters, true));
	}
	if(GetScannerConfig()->GitChecker.bGitCheck)
	{
		FString OutRepoDir;
		if(UFlibSourceControlHelper::FindRootDirectory(GetScannerConfig()->GitChecker.GetRepoDir(),OutRepoDir))
		{
			GlobalAssets.Append(UFlibAssetParseHelper::GetAssetsByObjectPath(UFlibAssetParseHelper::GetAssetsByGitChecker(GetScannerConfig()->GitChecker)));
		}
		else
		{
			UE_LOG(LogResScannerProxy,Display,TEXT("%s is not a valid git repo."),*OutRepoDir);
		}
	}

	if(GetScannerConfig()->bUseRulesTable)
	{
		TArray<FScannerMatchRule> ImportRules = GetScannerConfig()->GetTableRules();
		
		for(int32 RuleID = 0;RuleID < ImportRules.Num();++RuleID)
		{
			bool bCheck = GetScannerConfig()->bRuleWhiteList ? GetScannerConfig()->RuleWhileListIDs.Contains(RuleID) : true;
			if(bCheck)
			{
				ScanSingleRule(GlobalAssets,ImportRules[RuleID],RuleID);
			}
		}
	}
	
	for(int32 RuleID = 0;RuleID < GetScannerConfig()->ScannerRules.Num();++RuleID)
	{
		bool bCheck = GetScannerConfig()->bRuleWhiteList ? GetScannerConfig()->RuleWhileListIDs.Contains(RuleID) : true;
		if(bCheck)
		{
			ScanSingleRule(GlobalAssets,GetScannerConfig()->ScannerRules[RuleID],RuleID);
		}
	}

	FString Name = GetScannerConfig()->ConfigName;
	if(Name.IsEmpty())
	{
		Name = FDateTime::UtcNow().ToString();
	}
	FRuleMatchedInfo::ResetTransient();
	FRuleMatchedInfo::SetSerializeTransient(GetScannerConfig()->GitChecker.bGitCheck && GetScannerConfig()->GitChecker.bRecordCommiter);
	if(GetScannerConfig()->GitChecker.bRecordCommiter)
	{
		UFlibAssetParseHelper::CheckMatchedAssetsCommiter(MatchedResult,GetScannerConfig()->GitChecker.GetRepoDir());
	}
	FString SaveBasePath = UFlibAssetParseHelper::ReplaceMarkPath(GetScannerConfig()->SavePath.Path);
	// serialize config
	if(GetScannerConfig()->bSaveConfig)
	{
		FString SerializedJsonStr;
		TemplateHelper::TSerializeStructAsJsonString(*ScannerConfig,SerializedJsonStr);
		
		FString SavePath = FPaths::Combine(SaveBasePath,FString::Printf(TEXT("%s_config.json"),*Name));
		if(FFileHelper::SaveStringToFile(SerializedJsonStr, *SavePath,FFileHelper::EEncodingOptions::ForceUTF8) && !IsRunningCommandlet())
		{
			FText Msg = LOCTEXT("SavedScanConfigMag", "Successd to Export the Config.");
			UFlibAssetParseHelper::CreateSaveFileNotify(Msg,SavePath,SNotificationItem::CS_Success);
		}
	}
	FString ResultSavePath = FPaths::Combine(SaveBasePath,FString::Printf(TEXT("%s_result.json"),*Name));
	IFileManager::Get().Delete(*ResultSavePath);
	
	// serialize matched assets
	if(GetScannerConfig()->bSaveResult && MatchedResult.MatchedAssets.Num())
	{
		FString SerializedJsonStr = SerializeResult();
		if(FFileHelper::SaveStringToFile(SerializedJsonStr, *ResultSavePath,FFileHelper::EEncodingOptions::ForceUTF8) && !IsRunningCommandlet())
		{
			FText Msg = LOCTEXT("SavedScanResultMag", "Successd to Export the scan result.");
			if(::IsRunningCommandlet())
			{
				UE_LOG(LogResScannerProxy,Log,TEXT("%s"),*SerializedJsonStr);	
			}
			else
			{
				UFlibAssetParseHelper::CreateSaveFileNotify(Msg,ResultSavePath,SNotificationItem::CS_Success);
			}
		}
	}
}

void UResScannerProxy::SetScannerConfig(FScannerConfig InConfig)
{
	if(!ScannerConfig.IsValid())
	{
		ScannerConfig = MakeShareable(new FScannerConfig);
	}
	*ScannerConfig = InConfig;
}

FString UResScannerProxy::SerializeResult()
{
	FString OutString;
	if(GetScannerConfig()->bSavaeLiteResult)
	{
		OutString = SerializeLiteResult();
	}
	else
	{
		TemplateHelper::TSerializeStructAsJsonString(GetScanResult(),OutString);
	}
	return OutString;
}

FString UResScannerProxy::SerializeLiteResult()
{
	FString Result;
	bool bRecordCommiter = GetScannerConfig()->GitChecker.bGitCheck && GetScannerConfig()->GitChecker.bRecordCommiter;
	for(const auto& RuleMatchedInfo:MatchedResult.MatchedAssets)
	{
		Result += TEXT("-------------------------------------------\n");
		if(RuleMatchedInfo.AssetPackageNames.Num() || RuleMatchedInfo.AssetsCommiter.Num())
		{
			FString Describle = RuleMatchedInfo.RuleDescribe.IsEmpty() ? TEXT(""):FString::Printf(TEXT("(%s)"),*RuleMatchedInfo.RuleDescribe);
			Result += FString::Printf(TEXT("规则名: %s (%d) %s\n"),*RuleMatchedInfo.RuleName,RuleMatchedInfo.AssetPackageNames.Num(),*Describle);
		}
		
		if(bRecordCommiter)
		{
			if(RuleMatchedInfo.AssetsCommiter.Num())
			{
				for(const auto& AssetCommiter:RuleMatchedInfo.AssetsCommiter)
				{
					Result += FString::Printf(TEXT("\t%s, %s\n"),*AssetCommiter.File,*AssetCommiter.Commiter);
				}
			}
		}
		else
		{
			if(RuleMatchedInfo.AssetPackageNames.Num())
			{
				for(const auto& AssetPackageName:RuleMatchedInfo.AssetPackageNames)
				{
					Result += FString::Printf(TEXT("\t%s\n"),*AssetPackageName);
				}
			}
		}
	}
	return Result;
}

void UResScannerProxy::PostProcessorMatchRule(const FScannerMatchRule& Rule,const FRuleMatchedInfo& RuleMatchedInfo)
{
	for(const auto& PostProcessorClass:Rule.PostProcessors)
	{
		if(IsValid(PostProcessorClass))
		{
			UScannnerPostProcessorBase* PostProcessorIns = Cast<UScannnerPostProcessorBase>(PostProcessorClass->GetDefaultObject());
			if(PostProcessorIns)
			{
				PostProcessorIns->Processor(RuleMatchedInfo,Rule.ScanAssetType->GetName());
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE
