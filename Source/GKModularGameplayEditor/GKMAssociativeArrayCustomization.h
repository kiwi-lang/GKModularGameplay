#pragma once

// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Unreal Engine
#include "IPropertyTypeCustomization.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

class IPropertyHandle;


/*
 * Shows a struct with Key & Value property with a TMap like layout
 */
class FGKMAssociativeArrayCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() 
	{
		return MakeShareable(new FGKMAssociativeArrayCustomization);
	}

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle,
		class FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
       
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle,
		class IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
};
