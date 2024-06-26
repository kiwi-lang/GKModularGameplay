// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// include
#include "GKMExperienceDefinition.h"

//
#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GKMExperienceDefinition)

#define LOCTEXT_NAMESPACE "GKMSystem"


namespace {
	int _InsertionOrderCounter() {
		static int i = 0;
		return i++;
	}
}

FGKMGameFeatureActionSection::FGKMGameFeatureActionSection() {
	InsertionOrder = _InsertionOrderCounter();
}

UGKMExperienceDefinition::UGKMExperienceDefinition()
{
}

#if WITH_EDITOR
EDataValidationResult UGKMExperienceDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);


#define WITH_OLD_ACTIONS 0
#if WITH_OLD_ACTIONS
	int32 EntryIndex = 0;
	for (const UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			EDataValidationResult ChildResult = Action->IsDataValid(Context);
			Result = CombineDataValidationResults(Result, ChildResult);
		}
		else
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Actions"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}
#endif


	int32 EntryIndex = 0;
	for (FGKMGameFeatureActionSection const& Section : Sections)
	{
		for (FGKMGameFeatureAction const& ActionItem : Section.Value) {
			UGameFeatureAction const* Action = ActionItem.Value;

			if (Action)
			{
				EDataValidationResult ChildResult = Action->IsDataValid(Context);
				Result = CombineDataValidationResults(Result, ChildResult);
			}
			else
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Actions"), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	// Make sure users didn't subclass from a BP of this (it's fine and expected to subclass once in BP, just not twice)
	if (!GetClass()->IsNative())
	{
		const UClass* ParentClass = GetClass()->GetSuperClass();

		// Find the native parent
		const UClass* FirstNativeParent = ParentClass;
		while ((FirstNativeParent != nullptr) && !FirstNativeParent->IsNative())
		{
			FirstNativeParent = FirstNativeParent->GetSuperClass();
		}

		if (FirstNativeParent != ParentClass)
		{
			Context.AddError(FText::Format(LOCTEXT("ExperienceInheritenceIsUnsupported", "Blueprint subclasses of Blueprint experiences is not currently supported (use composition via ActionSets instead). Parent class was {0} but should be {1}."), 
				FText::AsCultureInvariant(GetPathNameSafe(ParentClass)),
				FText::AsCultureInvariant(GetPathNameSafe(FirstNativeParent))
			));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UGKMExperienceDefinition::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

#if WITH_OLD_ACTIONS
	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
#endif

	for (FGKMGameFeatureActionSection& Section : Sections)
	{
		for (FGKMGameFeatureAction& ActionItem : Section.Value) {
			UGameFeatureAction* Action = ActionItem.Value;
			if (Action) {
				Action->AddAdditionalAssetBundleData(AssetBundleData);
			}
		}
	}

}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE

#undef WITH_OLD_ACTIONS