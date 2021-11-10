#pragma once
#include "TemplateHelper.hpp"
// engine header
#include "AssetData.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/DataTable.h"
#include "FMatchRuleTypes.generated.h"

UENUM(BlueprintType)
enum class ENameMatchMode : uint8
{
	StartWith UMETA(DisplayName="以...开头"),
	EndWith UMETA(DisplayName="以...结尾"),
	Wildcard UMETA(DisplayName="通配符")
};

UENUM(BlueprintType)
enum class EPathMatchMode : uint8
{
	WithIn UMETA(DisplayName="位于路径中"),
	Wildcard UMETA(DisplayName="通配符"),
};

UENUM(BlueprintType)
enum class EMatchLogic : uint8
{
	Necessary UMETA(DisplayName="所有规则必须完全匹配"),
	Optional UMETA(DisplayName="所有规则中匹配一个"),
};

struct IMatchRule
{
	virtual ~IMatchRule(){}
	virtual EMatchLogic GetMatchLogic()const = 0;
};

USTRUCT(BlueprintType)
struct FTextRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="规则文本")
	FString RuleText;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="反转规则结果")
	bool bReverseCheck;
};

USTRUCT(BlueprintType)
struct FNameRule
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="命名匹配模式")
	ENameMatchMode MatchMode;
	// 匹配规则，是必须的还是可选的，Necessary是必须匹配所有的规则，Optional则只需要匹配规则中的一个
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="匹配逻辑")
	EMatchLogic MatchLogic;
	// UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(EditCondition="MatchLogic == EMatchLogic::Optional"))
	int32 OptionalRuleMatchNum = 1;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="路径匹配规则列表")
	TArray<FTextRule> Rules;
};

USTRUCT(BlueprintType)
struct FNameMatchRule
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="命名匹配规则")
	TArray<FNameRule> Rules;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="反转结果")
	bool bReverseCheck;
};

USTRUCT(BlueprintType)
struct FPathRule
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="路径匹配模式")
	EPathMatchMode MatchMode;
	// 匹配规则，是必须的还是可选的，Necessary是必须匹配所有的规则，Optional则只需要匹配规则中的一个
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="匹配逻辑")
	EMatchLogic MatchLogic;
	// UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(EditCondition="MatchLogic == EMatchLogic::Optional"))
	int32 OptionalRuleMatchNum = 1;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="规则列表",meta = (RelativeToGameContentDir, LongPackageName))
	TArray<FTextRule> Rules;
};

USTRUCT(BlueprintType)
struct FPathMatchRule
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="路径匹配规则列表")
	TArray<FPathRule> Rules;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="反转结果")
	bool bReverseCheck;

};

UENUM(BlueprintType)
enum class EPropertyMatchRule:uint8
{
	Equal UMETA(DisplayName="等于"),
	NotEqual UMETA(DisplayName="不等于")
	// LessThan UMETA(DisplayName="小于"),
	// GreatThan UMETA(DisplayName="大于")
};

USTRUCT(BlueprintType)
struct FRulePropertyMapping
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="属性名")
	FString PropertyName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="值")
	FString MatchValue;
};

USTRUCT(BlueprintType)
struct FPropertyMatchMapping
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="匹配模式")
	EPropertyMatchRule MatchRule;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="属性名")
	FString PropertyName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="值")
	FString MatchValue;
};

USTRUCT(BlueprintType)
struct FPropertyRule
{
	GENERATED_USTRUCT_BODY()
public:
	// 匹配规则，是必须的还是可选的，Necessary是必须匹配所有的规则，Optional则只需要匹配规则中的一个
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="匹配逻辑")
	EMatchLogic MatchLogic;
	// UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(EditCondition="MatchLogic == EMatchLogic::Optional"))
	int32 OptionalRuleMatchNum = 1;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="属性规则列表")
	TArray<FPropertyMatchMapping> Rules;
	
};
USTRUCT(BlueprintType)
struct FPropertyMatchRule
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="属性匹配规则列表")
	TArray<FPropertyRule> MatchRules;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="反转结果")
    bool bReverseCheck;
};

USTRUCT(BlueprintType)
struct FAssetFilters
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="扫描目录列表",meta = (RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> Filters;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="扫描资源列表")
	TArray<FSoftObjectPath> Assets;
};

// 规则的优先级程度
UENUM(BlueprintType)
enum class ERulePriority:uint8
{
	IMPORTENT UMETA(DisplayName="重要"),
	GENERAL  UMETA(DisplayName="一般"),
	LOW  UMETA(DisplayName="低")
};

USTRUCT(BlueprintType)
struct FFileCommiter
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString File;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString Commiter;
};

USTRUCT(BlueprintType)
struct FRuleMatchedInfo
{
	GENERATED_USTRUCT_BODY()
public:
	FRuleMatchedInfo():RuleName(TEXT("")),RuleDescribe(TEXT("")),RuleID(-1){}
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString RuleName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString RuleDescribe;
	// 规则的重要程度（优先级）
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	ERulePriority Priority = ERulePriority::GENERAL;
	// 该规则在配置数组中的下标
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 RuleID;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, transient)
	TArray<FAssetData> Assets;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<FString> AssetPackageNames;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<FFileCommiter> AssetsCommiter;

	static void SetSerializeTransient(bool bCommiter)
	{
		FString NotSerializeName = bCommiter ? TEXT("AssetPackageNames") : TEXT("AssetsCommiter");
		for(TFieldIterator<FProperty> PropertyIter(FRuleMatchedInfo::StaticStruct());PropertyIter;++PropertyIter)
		{
			FProperty* PropertyIns = *PropertyIter;
			if(NotSerializeName.Equals(*PropertyIns->GetName()))
			{
				PropertyIns->SetPropertyFlags(CPF_Transient);
			}
		}
	}
	
	static void ResetTransient()
	{
		TArray<FString> NotSerializeNames = {TEXT("AssetsCommiter"),TEXT("AssetPackageNames")};
		for(TFieldIterator<FProperty> PropertyIter(FRuleMatchedInfo::StaticStruct());PropertyIter;++PropertyIter)
		{
			FProperty* PropertyIns = *PropertyIter;
			if(NotSerializeNames.Contains(*PropertyIns->GetName()) && PropertyIns->HasAnyPropertyFlags(CPF_Transient))
			{
				PropertyIns->ClearPropertyFlags(CPF_Transient);
			}
		}
	}
};

UCLASS(Blueprintable,BlueprintType)
class UOperatorBase : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	bool Match(UObject* Object,const FString& AssetType);
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	EMatchLogic GetMatchLogic()const;

	virtual bool Match_Implementation(UObject* Object,const FString& AssetType){ return false; }
	virtual EMatchLogic GetMatchLogic_Implementation()const { return EMatchLogic::Necessary; };
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	EMatchLogic MatchLogic;
};

UCLASS(Blueprintable,BlueprintType)
class UScannnerPostProcessorBase : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	bool Processor(const FRuleMatchedInfo& MatchedInfo,const FString& AssetType);
};


USTRUCT(BlueprintType)
struct RESSCANNER_API FScannerMatchRule:public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	// 规则名
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="规则名",Category = "Filter")
	FString RuleName;
	// 规则描述
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="规则描述",Category = "Filter")
	FString RuleDescribe;
	// 是否启用当前规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="启用当前规则",Category = "Filter")
	bool bEnableRule = true;

	// 规则的重要程度（优先级）
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="规则的重要程度(优先级)",Category = "Filter")
	ERulePriority Priority = ERulePriority::GENERAL;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="必须匹配规则中的Filter目录(全局资源)",Category="Filter")
	bool bGlobalAssetMustMatchFilter = true;
	// 扫描资源路径
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="扫描资源路径",Category = "Filter",meta = (RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> ScanFilters;
	// 资源类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="扫描资源类型",Category = "Filter")
	UClass* ScanAssetType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="递归子类类型",Category = "Filter")
	bool RecursiveClasses = true;
	// 命名匹配规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="命名匹配",Category = "Filter")
	FNameMatchRule NameMatchRules;
	// 路径匹配规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="路径匹配",Category = "Filter")
	FPathMatchRule PathMatchRules;
	// 属性匹配规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="属性匹配",Category = "Filter")
	FPropertyMatchRule PropertyMatchRules;
	// 自定义匹配规则（派生自UOperatorBase的类）
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="自定义匹配规则",Category = "Filter")
	TArray<TSubclassOf<UOperatorBase>> CustomRules;
	// 忽略本规则的路径、资源列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="扫描时忽略的资源",Category = "Filter")
	FAssetFilters IgnoreFilters;
	// 是否开启匹配资源的后处理
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="启用资源扫描后处理",Category = "Filter")
	bool bEnablePostProcessor = false;
	// 当扫描完毕之后，对命中规则的资源进行处理
	UPROPERTY(EditAnywhere, BlueprintReadWrite,DisplayName="后处理规则",Category = "Filter",meta=(EditCondition="bEnablePostProcessor"))
	TArray<TSubclassOf<UScannnerPostProcessorBase>> PostProcessors;

	bool operator==(const FScannerMatchRule& R)const;
	
	bool HasValidRules()const { return (NameMatchRules.Rules.Num() || PathMatchRules.Rules.Num() || PropertyMatchRules.MatchRules.Num() || CustomRules.Num()); }
};

USTRUCT(BlueprintType)
struct RESSCANNER_API FGitChecker
{
	GENERATED_USTRUCT_BODY()
public:
	FGitChecker()
	{
		RepoDir.Path = TEXT("[PROJECT_CONTENT_DIR]");
	}
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="启用Git仓库扫描",Category="GitChecker")
	bool bGitCheck = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="记录提交人",Category="GitChecker",meta=(EditCondition="bGitCheck"))
	bool bRecordCommiter = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="Git仓库地址",Category="GitChecker",meta=(EditCondition="bGitCheck"))
	FDirectoryPath RepoDir;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="Git提交记录比对",Category="GitChecker",meta=(EditCondition="bGitCheck"))
	bool bDiffCommit = true;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="检查开始的Commit",Category="GitChecker|DiffCommit",meta=(EditCondition="bGitCheck && bDiffCommit"))
	FString BeginCommitHash = TEXT("HEAD~");
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="检查结束的Commit",Category="GitChecker|DiffCommit",meta=(EditCondition="bGitCheck && bDiffCommit"))
	FString EndCommitHash = TEXT("HEAD");
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="检查待提交文件",Category="GitChecker",meta=(EditCondition="bGitCheck && !bDiffCommit"))
	bool bUncommitFiles = false;
	
	// UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="记录文件的提交人",Category="GitChecker",meta=(EditCondition="bGitCheck"))
	// bool bRecordCommmiter = false;
	
	FString GetRepoDir()const;
};

USTRUCT(BlueprintType)
struct RESSCANNER_API FScannerConfig
{
	GENERATED_USTRUCT_BODY()
public:
	FORCEINLINE static FScannerConfig* Get()
	{
		static FScannerConfig StaticIns;
		return &StaticIns;
	}
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="配置名",Category="Base")
	FString ConfigName;
	// if true,only scan the GlobalScanFilters assets
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="启用全局资源",Category="Global")
	bool bByGlobalScanFilters = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="屏蔽每个规则中配置的资源",Category="Global",meta=(EditCondition="bByGlobalScanFilters"))
	bool bBlockRuleFilter = false;
	// if bByGlobalFilters is true,all rule using the filter assets
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="全局扫描配置",Category="Global",meta=(EditCondition="bByGlobalScanFilters"))
	FAssetFilters GlobalScanFilters;
	// force ignore assets,don't match any rule
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="全局忽略扫描配置",Category="Global",meta=(EditCondition="bByGlobalScanFilters"))
	TArray<FAssetFilters> GlobalIgnoreFilters;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="Git仓库扫描配置",Category="Global",meta=(EditCondition="bByGlobalScanFilters"))
	FGitChecker GitChecker;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="开启规则白名单",Category="WhiteList")
	bool bRuleWhiteList = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="规则白名单（名单中的规则才会被执行）",Category="WhiteList",meta=(EditCondition="bRuleWhiteList"))
	TArray<int32> RuleWhileListIDs;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="启用规则数据表",Category="RulesTable")
	bool bUseRulesTable = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="规则数据表",Category="RulesTable", meta=(AllowedClasses="DataTable",RequiredAssetDataTags = "RowStructure=ScannerMatchRule",EditCondition="bUseRulesTable"))
	FSoftObjectPath ImportRulesTable;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="规则列表",Category="Rules")
	TArray<FScannerMatchRule> ScannerRules;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="存储配置文件",Category="Save")
	bool bSaveConfig = true;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="存储扫描结果",Category="Save")
	bool bSaveResult = true;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="简洁扫描结果",Category="Save",meta=(EditCondition="bSaveResult"))
	bool bSavaeLiteResult = true;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="存储路径",Category="Save")
	FDirectoryPath SavePath;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName="独立运行模式",Category="Advanced")
	bool bStandaloneMode = true;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Advanced")
	FString AdditionalExecCommand;

	TArray<FScannerMatchRule> GetTableRules()const;
	void HandleImportRulesTable();
};

USTRUCT(BlueprintType)
struct FMatchedRuleAsset
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString OperatorRuleName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, transient)
	TArray<FAssetData> Assets;
};


USTRUCT(BlueprintType)
struct FMatchedResult
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<FRuleMatchedInfo> MatchedAssets;
};
