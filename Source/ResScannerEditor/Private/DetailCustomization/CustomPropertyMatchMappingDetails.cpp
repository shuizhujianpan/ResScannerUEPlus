#include "CustomPropertyMatchMappingDetails.h"
#include "FMatchRuleTypes.h"
// engine header
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Resources/Version.h"
#define LOCTEXT_NAMESPACE "CustomPropertyMatchMappingDetails"

void FCustomPropertyMatchMappingDetails::RefreshCustomizeHead()
{
    auto CreatePropertyWidgetLambda = [this](TSharedPtr<SComboBox<TSharedPtr<FString>>>& ComboBox,const TArray<TSharedPtr<FString>>& ComboxItems,const TSharedPtr<IPropertyHandle>& PropertyHandle,TSharedPtr<SBox>& WidgetArea)
    {
        FString PropertyName;
        PropertyHandle->GetValue(PropertyName);
        if(PropertyName.IsEmpty())
        {
            PropertyName = *ComboxItems[0].Get();
        }
        ComboBox->SetSelectedItem(MakeShareable(new FString(PropertyName)));
        TSharedRef<SWidget> Widget = SNew(STextBlock).Text(FText::FromString(PropertyName));
        WidgetArea->SetContent(Widget);
    };
    if(AllPropertyNames.Num())
        CreatePropertyWidgetLambda(AllPropertySelector,AllPropertyNames,PropertyNameHandle,PropertyNameComboContent);
}


TSharedRef<SWidget> FCustomPropertyMatchMappingDetails::HandleGenerateWidget_ForPropertyNamesComboBox(TSharedPtr<FString> Item) const
{
    FString StringItem = Item.IsValid() ? *Item : FString();

    // If a row wasn't generated just create the default one, a simple text block of the item's name.
    return SNew(STextBlock)
        .Text(FText::FromString(StringItem));
}
void FCustomPropertyMatchMappingDetails::OnPropertyValueChanged()
{
    if(CurrentPropertyRow)
    {
        FString Value;
        CurrentPropertyRow->GetPropertyHandle()->GetValueAsFormattedString(Value);
        MatchValueHandle->SetValueFromFormattedString(Value);
    }
}

void FCustomPropertyMatchMappingDetails::OnClassValueChanged()
{
    UObject* ClassObject = NULL;
    Class = NULL;
    IDetailCategoryBuilder& ParentCategory = mStructBuilder->GetParentCategory();
    IDetailLayoutBuilder& ParentLayout = ParentCategory.GetParentLayout();
    auto ScanAssetType = ParentLayout.GetProperty(GET_MEMBER_NAME_CHECKED(FScannerMatchRule, ScanAssetType));
    
    ScanAssetType->GetValue(ClassObject);
    if(ClassObject)
    {
        Class = Cast<UClass>(ClassObject);
        UE_LOG(LogTemp,Log,TEXT("Property Name %s"),*Class->GetName());
        for(TFieldIterator<FProperty> PropertyIter(Class);PropertyIter;++PropertyIter)
        {
            FProperty* PropertyIns = *PropertyIter;
            UE_LOG(LogTemp,Log,TEXT("Property Name: %s"),*PropertyIns->GetName());
            AllPropertyNames.Add(MakeShareable(new FString(PropertyIns->GetName())));
            AllPropertyMap.Add(PropertyIns->GetName(),PropertyIns);
        }
    }
    TSharedPtr<SWidget> Widget;
    if(Class)
    {
        SAssignNew(AllPropertySelector,SComboBox<TSharedPtr<FString>>)
        .OptionsSource(&AllPropertyNames)
        .OnGenerateWidget(SComboBox<TSharedPtr<FString>>::FOnGenerateWidget::CreateRaw(this,&FCustomPropertyMatchMappingDetails::HandleGenerateWidget_ForPropertyNamesComboBox))
        .OnSelectionChanged(SComboBox< TSharedPtr<FString> >::FOnSelectionChanged::CreateRaw(this,&FCustomPropertyMatchMappingDetails::HandleSelectionChanged_ForPropertyNamesComboBox))
        [
            SAssignNew(PropertyNameComboContent, SBox)
        ];
        PropertyNameComboContent->SetContent(SNew(STextBlock).Text(FText::FromString(*AllPropertyNames[0].Get())));
        Widget = AllPropertySelector;
    }
    else
    {
        Widget = PropertyNameHandle->CreatePropertyValueWidget();
    }
    AllPropertySelectorBox->SetContent(Widget.ToSharedRef());
}

void FCustomPropertyMatchMappingDetails::HandleSelectionChanged_ForPropertyNamesComboBox(TSharedPtr<FString> Item, ESelectInfo::Type SelectionType)
{
    TSharedPtr<FString> StringItem = AllPropertySelector->GetSelectedItem();
    if(StringItem.IsValid())
    {
        TSharedRef<SWidget> ComboContentText = SNew(STextBlock).Text(FText::FromString(*StringItem.Get()));
        PropertyNameComboContent->SetContent(ComboContentText);
        PropertyNameHandle->SetValue(*StringItem.Get());
    }

    if(Class && StringItem.IsValid())
    {
        FProperty* PropertyIns = NULL;
        for(TFieldIterator<FProperty> PropertyIter(Class);PropertyIter;++PropertyIter)
        {
            if(PropertyIter->GetName().Equals(*StringItem.Get()))
            {
                PropertyIns = *PropertyIter;
            }
        }
        if(PropertyIns)
        {
            FString PropertyName = *StringItem.Get();
            IDetailPropertyRow* PropertyRow = mStructBuilder->AddExternalStructureProperty(MakeShareable(new FStructOnScope(Cast<UStruct>(Class))), *PropertyName);

            // reset Property value to default
            if(OldPropertyRow || !PropertyRow)
            {
                MatchValueHandle->SetValueFromFormattedString(TEXT(""));
            }
            
            TSharedRef<SWidget> Widget = MatchValueHandle->CreatePropertyValueWidget();
            if(PropertyRow)
            {
                OldPropertyRow = CurrentPropertyRow;
                CurrentPropertyRow = PropertyRow;
                PropertyRow->Visibility(EVisibility::Hidden);
                FSimpleDelegate OnPropertyValueChanged = FSimpleDelegate::CreateSP(this, &FCustomPropertyMatchMappingDetails::OnPropertyValueChanged);
                PropertyRow->GetPropertyHandle()->SetOnPropertyValueChanged(OnPropertyValueChanged);
                
                FString ExistValue;
                MatchValueHandle->GetValueAsFormattedString(ExistValue);
                PropertyRow->GetPropertyHandle()->SetValueFromFormattedString(ExistValue);
                
                Widget = PropertyRow->GetPropertyHandle()->CreatePropertyValueWidget();
                

            }
            
            PropertyContent->SetContent(Widget);
        }
    }
}

void FCustomPropertyMatchMappingDetails::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
                                                         FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    MatchRuleHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPropertyMatchMapping, MatchRule));
    MatchValueHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPropertyMatchMapping, MatchValue));
    PropertyNameHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPropertyMatchMapping, PropertyName));
    CurrentPropertyRow = NULL;
    OldPropertyRow = NULL;
    HeaderRow.NameContent()		//属性的Name样式
    [
        SNew(STextBlock)
        .Text(FText::FromString(TEXT("属性规则")))
        ];
    
}

void FCustomPropertyMatchMappingDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
    IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    mStructBuilder = &StructBuilder;
    IDetailCategoryBuilder& ParentCategory = mStructBuilder->GetParentCategory();
    IDetailLayoutBuilder& ParentLayout = ParentCategory.GetParentLayout();
    auto ScanAssetType = ParentLayout.GetProperty(GET_MEMBER_NAME_CHECKED(FScannerMatchRule, ScanAssetType));
    ScanAssetType->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FCustomPropertyMatchMappingDetails::OnClassValueChanged));
    
    StructBuilder.AddCustomRow(FText::FromString(TEXT("PropertyName")))
         .NameContent()
         [
           PropertyNameHandle->CreatePropertyNameWidget()
         ]
         .ValueContent()
         .MinDesiredWidth(500)
         [
            SAssignNew(AllPropertySelectorBox, SBox)
         ];
    
    OnClassValueChanged();

    
    StructBuilder.AddCustomRow(FText::FromString(TEXT("MatchRule")))
        .NameContent()
        [
          MatchRuleHandle->CreatePropertyNameWidget()
        ]
        .ValueContent()
        .MinDesiredWidth(500)
        [
            MatchRuleHandle->CreatePropertyValueWidget()
        ];
    StructBuilder.AddCustomRow(FText::FromString(TEXT("PropertyValue")))
        .NameContent()
        [
          MatchValueHandle->CreatePropertyNameWidget()
        ]
        .ValueContent()
        .MinDesiredWidth(500)
        [
            SAssignNew(PropertyContent,SBox)
        ];
    PropertyContent->SetContent(MatchValueHandle->CreatePropertyValueWidget());
    RefreshCustomizeHead();
    
}
#undef LOCTEXT_NAMESPACE
