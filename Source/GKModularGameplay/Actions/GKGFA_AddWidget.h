// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//
#include "GKGFA_WorldActionBase.h"

//
#include "CommonActivatableWidget.h"

#include "UIExtensionSystem.h"

// Generated
#include "GKGFA_AddWidget.generated.h"

struct FWorldContext;
struct FComponentRequestHandle;

USTRUCT()
struct FGKMHUDLayoutRequest
{
	GENERATED_BODY()

	// The layout widget to spawn
	UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	// The layer to insert the widget in
	UPROPERTY(EditAnywhere, Category=UI, meta=(Categories="UI.Layer"))
	FGameplayTag LayerID;
};


USTRUCT()
struct FGKMHUDElementEntry
{
	GENERATED_BODY()

	FGKMHUDElementEntry();

	// The widget to spawn
	UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	// The slot ID where we should place this widget
	UPROPERTY(EditAnywhere, Category = UI)
	FGameplayTag SlotID;

	//! Remove previous extension if exists
	UPROPERTY(EditAnywhere, Category = UI)
	bool bUnique;
};

//////////////////////////////////////////////////////////////////////
// UGKGFA_AddWidget

/**
 * GameFeatureAction responsible for granting abilities (and attributes) to actors of a specified type.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Widgets"))
class UGKGFA_AddWidgets final : public UGKGFA_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~ End UGameFeatureAction interface

	//~ Begin UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject interface

private:
	// Layout to add to the HUD
	UPROPERTY(EditAnywhere, Category=UI, meta=(TitleProperty="{LayerID} -> {LayoutClass}"))
	TArray<FGKMHUDLayoutRequest> Layout;

	// Widgets to add to the HUD
	UPROPERTY(EditAnywhere, Category=UI, meta=(TitleProperty="{SlotID} -> {WidgetClass}"))
	TArray<FGKMHUDElementEntry> Widgets;

private:

	struct FPerActorData
	{
		TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
		TArray<FUIExtensionHandle> ExtensionHandles;
		TMap<FGameplayTag, FUIExtensionHandle> ExtendedEntryPoints;
	};

	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
		TMap<FObjectKey, FPerActorData> ActorData; 
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	//~ Begin UGKGFA_WorldActionBase interface
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End UGKGFA_WorldActionBase interface

	void Reset(FPerContextData& ActiveData);

	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	void AddWidgets(AActor* Actor, FPerContextData& ActiveData);
	void RemoveWidgets(AActor* Actor, FPerContextData& ActiveData);
};
