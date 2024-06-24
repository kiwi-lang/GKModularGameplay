// Include
#include "GKMAssociativeArrayCustomization.h"


//
#include "GKModularGameplayEdLog.h"

// Unreal Engine
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "PropertyHandle.h"
#include "PropertyCustomizationHelpers.h"
#include "Templates/SharedPointer.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "DetailCategoryBuilder.h"
#include "GameFeatureAction.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
// #include "PropertyHandleImpl.h"


void FGKMAssociativeArrayCustomization::CustomizeHeader(
    TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> Key = InStructPropertyHandle->GetChildHandle(FName("Key"));
	TSharedPtr<IPropertyHandle> Value = InStructPropertyHandle->GetChildHandle(FName("Value"));

	// Setup in the header row so that we still get the TArray dropdown
	HeaderRow
		.NameContent()
		[
			Key->CreatePropertyValueWidget()
		]
		.ValueContent()
		.MaxDesiredWidth(0.0f)
		[
			Value->CreatePropertyValueWidget()
		];

	// This avoids making duplicate reset boxes
    InStructPropertyHandle->MarkResetToDefaultCustomized();
}


// Flatten the struct tree to make it more readable
void Recurse(IDetailChildrenBuilder& StructBuilder, TSharedPtr<IPropertyHandle> Property, int Depth, int Limit)
{
	uint32 NumElements = 0;
	Property->GetNumChildren(NumElements);

	// Array should not be flatten even if they have only one element
	bool bIsArray = Property->AsArray().IsValid();

	// Flatten struct with one element only
	bool bFlattenSmallStruct = NumElements == 1;

	// Avoid Recurse too deep
	bool bShouldRecurse = (Depth < Limit || bFlattenSmallStruct);

	// only recurse when not array
	bool bWillRecurse = !bIsArray && bShouldRecurse;

	if (bWillRecurse)
	{ 
		for (uint32 Index = 0; Index < NumElements; ++Index)
		{
			TSharedPtr<IPropertyHandle> ChildProperty = Property->GetChildHandle(Index);
			Recurse(StructBuilder, ChildProperty, Depth + 1, Limit);
		}
	}
	else {
		StructBuilder.AddProperty(Property.ToSharedRef());
	}
}

void FGKMAssociativeArrayCustomization::CustomizeChildren(
    TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> ActionsProperty = InStructPropertyHandle->GetChildHandle(FName("Value"));

    // Add Actions property as an array
    if (ActionsProperty.IsValid() && ActionsProperty->IsValidHandle())
    {
        // Get the number of elements in the array
		if (ActionsProperty->AsArray().IsValid()) {
			uint32 NumElements = 0;
			ActionsProperty->GetNumChildren(NumElements);

			for (uint32 Index = 0; Index < NumElements; ++Index)
			{
				TSharedPtr<IPropertyHandle> ElementProperty = ActionsProperty->GetChildHandle(Index);

				if (ElementProperty.IsValid())
				{
					StructBuilder.AddProperty(ElementProperty.ToSharedRef());
				}
			}
		} else {
			// Here Num children == 1, that's the instanced object
			Recurse(StructBuilder, ActionsProperty,0, 2);
		}
    }
}
