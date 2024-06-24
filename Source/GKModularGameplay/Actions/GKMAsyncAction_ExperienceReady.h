// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

// Generated
#include "GKMAsyncAction_ExperienceReady.generated.h"

class AGameStateBase;
class ULyraExperienceDefinition;
class UWorld;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGKExperienceReadyAsyncDelegate);

/**
 * Asynchronously waits for the game state to be ready and valid and then calls the OnReady event.  Will call OnReady
 * immediately if the game state is valid already.
 */
UCLASS()
class UGKMAsyncAction_ExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	// Waits for the experience to be determined and loaded
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UGKMAsyncAction_ExperienceReady* WaitForExperienceReady(UObject* WorldContextObject);

	virtual void Activate() override;

public:

	// Called when the experience has been determined and is ready/loaded
	UPROPERTY(BlueprintAssignable)
	FGKExperienceReadyAsyncDelegate OnReady;

private:
	void Step1_HandleGameStateSet(AGameStateBase* GameState);
	void Step2_ListenToExperienceLoading(AGameStateBase* GameState);
	void Step3_HandleExperienceLoaded(const UGKMExperienceDefinition* CurrentExperience);
	void Step4_BroadcastReady();

	TWeakObjectPtr<UWorld> WorldPtr;
};
