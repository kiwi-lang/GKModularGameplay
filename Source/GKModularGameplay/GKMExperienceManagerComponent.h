// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

//
#include "Components/GameStateComponent.h"
#include "LoadingProcessInterface.h"

// Genetated
#include "GKMExperienceManagerComponent.generated.h"

namespace UE::GameFeatures { struct FResult; }

class UGKMExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FGKOnExperienceLoaded, const UGKMExperienceDefinition* /*Experience*/);

enum class EGKMExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

/*
 * The original class only load a single experience 
 */
UCLASS()
class UGKMExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:

	UGKMExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	//~ILoadingProcessInterface interface
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface

	// Tries to set the current experience, either a UI or gameplay one
	void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	// Ensures the delegate is called once the experience has been loaded,
	// before others are called.
	// However, if the experience has already loaded, calls the delegate immediately.
	void CallOrRegister_OnExperienceLoaded_HighPriority(FGKOnExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded(FGKOnExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded_LowPriority(FGKOnExperienceLoaded::FDelegate&& Delegate);

	// This returns the current experience if it is fully loaded, asserting otherwise
	// (i.e., if you called it too soon)
	const UGKMExperienceDefinition* GetCurrentExperienceChecked() const;

	// Returns true if the experience is fully loaded
	bool IsExperienceLoaded() const;

private:
	UFUNCTION()
	void OnRep_CurrentExperience();

	UFUNCTION()
	void OnRep_Barrier();

	void StartExperienceLoad();
	void OnExperienceLoadComplete();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnExperienceFullLoadCompleted();

	void OnActionDeactivationCompleted();
	void OnAllActionsDeactivated();

	void ExperienceLoaded_High();
	void ExperienceLoaded_Normal();
	void ExperienceLoaded_Low();

private:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const UGKMExperienceDefinition> CurrentExperience;

	EGKMExperienceLoadState LoadState = EGKMExperienceLoadState::Unloaded;

	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;

	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Barrier)
	int Barrier;

	UPROPERTY(Transient)
	bool bHigh;

	UPROPERTY(Transient)
	bool bNormal;

	UPROPERTY(Transient)
	bool bLow;

	/**
	 * Delegate called when the experience has finished loading just before others
	 * (e.g., subsystems that set up for regular gameplay)
	 */
	FGKOnExperienceLoaded OnExperienceLoaded_HighPriority;

	/** Delegate called when the experience has finished loading */
	FGKOnExperienceLoaded OnExperienceLoaded;

	/** Delegate called when the experience has finished loading */
	FGKOnExperienceLoaded OnExperienceLoaded_LowPriority;
};
