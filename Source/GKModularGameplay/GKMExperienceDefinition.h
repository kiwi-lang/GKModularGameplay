// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once


// Unreal Engine
#include "Engine/DataAsset.h"
#include "Containers/SortedMap.h"

// Generated
#include "GKMExperienceDefinition.generated.h"

class UGameFeatureAction;


//! GameFeature action with a name
USTRUCT(BlueprintType)
struct FGKMGameFeatureAction
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "Actions")
	FString Key;
#endif

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Actions")
	TObjectPtr<UGameFeatureAction> Value;
};

//! Named Group of actions to execute
USTRUCT(BlueprintType)
struct FGKMGameFeatureActionSection
{
	GENERATED_BODY()

	FGKMGameFeatureActionSection();

	int InsertionOrder;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "Actions")
	FString Key;

#endif
	UPROPERTY(EditDefaultsOnly, Category = "Actions")
	TArray<FGKMGameFeatureAction> Value;
};



// class UGKMPawnData;
// class UGKMExperienceActionSet;


/**
 * Definition of an experience
 */
UCLASS(BlueprintType, Const)
class UGKMExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UGKMExperienceDefinition();

	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

public:
	// TODO Move this as a UGameFeatureAction
	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;

	/** The default pawn class to spawn for players */
	//@TODO: Make soft?
	// UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	// TObjectPtr<const UGKMPawnData> DefaultPawnData;

	// Why we already have Features & Actions
	// List of additional action sets to compose into this experience
	// UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	// TArray<TObjectPtr<UGKMExperienceActionSet>> ActionSets;


	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	// UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	// TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// Feature Tree to enable
	UPROPERTY(EditDefaultsOnly, Category = "Actions")
	TArray<FGKMGameFeatureActionSection> Sections;
};
