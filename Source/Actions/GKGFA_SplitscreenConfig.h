// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GKGFA_WorldActionBase.h"
#include "UObject/ObjectKey.h"

// Generated
#include "GKGFA_SplitscreenConfig.generated.h"

class UObject;
struct FGameFeatureDeactivatingContext;
struct FGameFeatureStateChangeContext;
struct FWorldContext;

//////////////////////////////////////////////////////////////////////
// UGKGFA_SplitscreenConfig

/**
 * GameFeatureAction responsible for granting abilities (and attributes) to actors of a specified type.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Splitscreen Config"))
class UGKGFA_SplitscreenConfig final : public UGKGFA_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction interface

	//~ Begin UGKGFA_WorldActionBase interface
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End UGKGFA_WorldActionBase interface

public:
	UPROPERTY(EditAnywhere, Category=Action)
	bool bDisableSplitscreen = true;

private:
	TArray<FObjectKey> LocalDisableVotes;

	static TMap<FObjectKey, int32> GlobalDisableVotes;
};
