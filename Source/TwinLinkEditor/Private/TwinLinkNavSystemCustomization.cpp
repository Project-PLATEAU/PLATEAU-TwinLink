// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkNavSystemCustomization.h"

TSharedRef<IPropertyTypeCustomization> FTwinLinkNavSystemCustomization::MakeInstance()
{
    return MakeShareable(new FTwinLinkNavSystemCustomization());
}

void FTwinLinkNavSystemCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
    FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
   
}

void FTwinLinkNavSystemCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
    IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
}
