#include "ScannerSettingsDetails.h"
#include "FMatchRuleTypes.h"
// engine header
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "ScannerSettingsDetails"

TSharedRef<IDetailCustomization> FScannerSettingsDetails::MakeInstance()
{
    return MakeShareable(new FScannerSettingsDetails());
}
#include "Resources/Version.h"
void FScannerSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray< TSharedPtr<FStructOnScope> > StructBeingCustomized;
    DetailBuilder.GetStructsBeingCustomized(StructBeingCustomized);
    check(StructBeingCustomized.Num() == 1);
    
    FScannerConfig* ScannerSettingsIns = (FScannerConfig*)StructBeingCustomized[0].Get()->GetStructMemory();

    IDetailCategoryBuilder* RulesCategory = NULL;
    {
        TArray<FName> Names;
        DetailBuilder.GetCategoryNames(Names);
        for(const auto& Name:Names)
        {
            IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(Name,FText::GetEmpty(),ECategoryPriority::Default);
            if(Name.IsEqual("RulesTable"))
            {
                RulesCategory = &Category;
            }
        }
    }
    if(!RulesCategory)
        return;

    // auto GlobalScanFilters = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FScannerConfig, GlobalScanFilters));
    // RulesCategory->AddProperty(GlobalScanFilters);
    RulesCategory->SetShowAdvanced(true);
    RulesCategory->AddCustomRow(LOCTEXT("ImportRulesTable", "Import Rules Table"),true)
        .ValueContent()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .Padding(0)
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("Import", "Import"))
                .ToolTipText(LOCTEXT("ImportRulesTable_Tooltip", "Import Rules Table to the Config"))
                .IsEnabled_Lambda([this,ScannerSettingsIns]()->bool
                {
                    return ScannerSettingsIns->bUseRulesTable;
                })
                .OnClicked_Lambda([this, ScannerSettingsIns]()
                {
                    if (ScannerSettingsIns)
                    {
                        ScannerSettingsIns->HandleImportRulesTable();
                    }
                    return(FReply::Handled());
                })
            ]
        ];
}
#undef LOCTEXT_NAMESPACE