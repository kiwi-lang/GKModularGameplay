// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKModularGameplayEd.h"

// Gamekit
#include "GKMAssociativeArrayCustomization.h"

// Unreal Engine
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

#define LOCTEXT_NAMESPACE "FGKModularGameplayEdModule"

void FGKModularGameplayEdModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomPropertyTypeLayout(
		"GKMGameFeatureActionSection",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGKMAssociativeArrayCustomization::MakeInstance)
	);
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"GKMGameFeatureAction",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGKMAssociativeArrayCustomization::MakeInstance)
	);

#if 0
	PropertyModule.RegisterCustomClassLayout(
		"GameFeatureAction",
		FOnGetDetailCustomizationInstance::CreateStatic(&FGKMGameFeatureActionCustomization::MakeInstance)
	);
#endif

#if 0
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"GKMGameFeatureAction",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGKMGameFeatureActionCustomization::MakeInstance)
	);
#endif
}

void FGKModularGameplayEdModule::ShutdownModule()
{
	FModuleManager::Get().OnModulesChanged().RemoveAll(this);

	// Unregister customization and callback
	FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");

	if (PropertyEditorModule)
	{
		PropertyEditorModule->UnregisterCustomPropertyTypeLayout(TEXT("GKMGameFeatureActionSection"));
		PropertyEditorModule->UnregisterCustomPropertyTypeLayout(TEXT("GKMGameFeatureAction"));
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGKModularGameplayEdModule, GKModularGameplayEd)