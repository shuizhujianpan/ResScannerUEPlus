// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class FCustomPropertyMatchMappingDetails : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance()
    {
        return MakeShareable(new FCustomPropertyMatchMappingDetails);
    }

    virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    void RefreshCustomizeHead();
    TSharedRef<SWidget> HandleGenerateWidget_ForPropertyNamesComboBox(TSharedPtr<FString> Item) const;
    void HandleSelectionChanged_ForPropertyNamesComboBox(TSharedPtr<FString> Item, ESelectInfo::Type SelectionType);
    
    void OnPropertyValueChanged();
    void OnClassValueChanged();

    TArray<TSharedPtr<FString>> AllPropertyNames;

    TSharedPtr<IPropertyHandle> MatchRuleHandle;
    TSharedPtr<IPropertyHandle> PropertyNameHandle;
    TSharedPtr<IPropertyHandle> MatchValueHandle;

    TSharedPtr<SBox> AllPropertySelectorBox;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> AllPropertySelector;
    TSharedPtr<SBox> PropertyNameComboContent;
    TSharedPtr<SBox> PropertyContent;
    TMap<FString,FProperty*> AllPropertyMap;

    UClass* Class;
    IDetailChildrenBuilder* mStructBuilder;
    class IDetailPropertyRow* CurrentPropertyRow;
    class IDetailPropertyRow* OldPropertyRow;
};