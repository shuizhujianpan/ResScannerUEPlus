#include "FMatchRuleTypes.h"

#include "FlibAssetParseHelper.h"

bool FScannerMatchRule::operator==(const FScannerMatchRule& R)const
{
	return RuleName.Equals(R.RuleName) && bEnableRule == R.bEnableRule && Priority == R.Priority && ScanAssetType == R.ScanAssetType;
}

FString FGitChecker::GetRepoDir() const
{
	return UFlibAssetParseHelper::ReplaceMarkPath(RepoDir.Path);
}

TArray<FScannerMatchRule> FScannerConfig::GetTableRules() const
{
	TArray<FScannerMatchRule> result;
	if(ImportRulesTable.IsValid())
	{
		ImportRulesTable.ToSoftObjectPath().TryLoad();
		UDataTable* RulesTable = ImportRulesTable.Get();
		if(RulesTable)
		{
			TArray<FName> RowNames = RulesTable->GetRowNames();
			FString ContextString;
			for (auto& name : RowNames)
			{
				FScannerMatchRule* pRow = RulesTable->FindRow<FScannerMatchRule>(name, ContextString);
				result.AddUnique(*pRow);
			}
		}
	}
	
	return result;
}

void FScannerConfig::HandleImportRulesTable()
{
	for (auto& Rule : GetTableRules())
	{
		ScannerRules.Add(Rule);
	}
}
