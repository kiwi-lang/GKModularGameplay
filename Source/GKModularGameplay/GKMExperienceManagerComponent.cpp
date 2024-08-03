// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.


// Include
#include "GKMExperienceManagerComponent.h"

// GK Modular Gameplay
#include "GKMLog.h"
#include "GKMExperienceDefinition.h"
#include "GKMAssetManager.h"
// #include "GKMExperienceActionSet.h"
#include "GKMExperienceManager.h"

// #include "System/GKMAssetManager.h"
// #include "GKMLogChannels.h"
// #include "Settings/GKMSettingsLocal.h"

// Unreal Engine
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystemSettings.h"
#include "TimerManager.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GKMExperienceManagerComponent)

//@TODO: Async load the experience definition itself
//@TODO: Handle failures explicitly (go into a 'completed but failed' state rather than check()-ing)
//@TODO: Do the action phases at the appropriate times instead of all at once
//@TODO: Support deactivating an experience and do the unloading actions
//@TODO: Think about what deactivation/cleanup means for preloaded assets
//@TODO: Handle deactivating game features, right now we 'leak' them enabled
// (for a client moving from experience to experience we actually want to diff the requirements and only unload some, not unload everything for them to just be immediately reloaded)
//@TODO: Handle both built-in and URL-based plugins (search for colon?)

namespace GKMConsoleVariables
{
	static float ExperienceLoadRandomDelayMin = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayMin(
		TEXT("GKM.chaos.ExperienceDelayLoad.MinSecs"),
		ExperienceLoadRandomDelayMin,
		TEXT("This value (in seconds) will be added as a delay of load completion of the experience (along with the random value GKM.chaos.ExperienceDelayLoad.RandomSecs)"),
		ECVF_Default);

	static float ExperienceLoadRandomDelayRange = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayRange(
		TEXT("GKM.chaos.ExperienceDelayLoad.RandomSecs"),
		ExperienceLoadRandomDelayRange,
		TEXT("A random amount of time between 0 and this value (in seconds) will be added as a delay of load completion of the experience (along with the fixed value GKM.chaos.ExperienceDelayLoad.MinSecs)"),
		ECVF_Default);

	float GetExperienceLoadDelayDuration()
	{
		return FMath::Max(0.0f, ExperienceLoadRandomDelayMin + FMath::FRand() * ExperienceLoadRandomDelayRange);
	}
}

UGKMExperienceManagerComponent::UGKMExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	Barrier = 0;
	bHigh = false;
	bNormal = false;
	bLow = false;
}

void UGKMExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{
	UGKMAssetManager& AssetManager = UGKMAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	TSubclassOf<UGKMExperienceDefinition> AssetClass = Cast<UClass>(AssetPath.TryLoad());
	check(AssetClass);
	const UGKMExperienceDefinition* Experience = GetDefault<UGKMExperienceDefinition>(AssetClass);

	check(Experience != nullptr);
	check(CurrentExperience == nullptr);
	CurrentExperience = Experience;
	StartExperienceLoad();
}

void UGKMExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(FGKOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
	}
}

void UGKMExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(FGKOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate));
	}
}

void UGKMExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(FGKOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
	}
}

const UGKMExperienceDefinition* UGKMExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	check(LoadState == EGKMExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

bool UGKMExperienceManagerComponent::IsExperienceLoaded() const
{
	return (LoadState == EGKMExperienceLoadState::Loaded) && (CurrentExperience != nullptr);
}

void UGKMExperienceManagerComponent::OnRep_CurrentExperience()
{
	StartExperienceLoad();
}

void UGKMExperienceManagerComponent::StartExperienceLoad()
{
	check(CurrentExperience != nullptr);
	check(LoadState == EGKMExperienceLoadState::Unloaded);

	GKMGP_LOG(TEXT("EXPERIENCE: StartExperienceLoad(CurrentExperience = %s, %s)"),
		*CurrentExperience->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));

	LoadState = EGKMExperienceLoadState::Loading;

	UGKMAssetManager& AssetManager = UGKMAssetManager::Get();

	TSet<FPrimaryAssetId> BundleAssetList;
	TSet<FSoftObjectPath> RawAssetList;

	/*
	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());
	for (const TObjectPtr<UGKMExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}
	*/

	// Load assets associated with the experience
	TArray<FName> BundlesToLoad;
	BundlesToLoad.Add(FGKMBundles::Equipped);

	//@TODO: Centralize this client/server stuff into the GKMAssetManager
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer);
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);
	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array(), BundlesToLoad, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}

	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	if (RawAssetList.Num() > 0)
	{
		RawLoadHandle = AssetManager.LoadAssetList(RawAssetList.Array(), FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, TEXT("StartExperienceLoad()"));
	}

	// If both async loads are running, combine them
	TSharedPtr<FStreamableHandle> Handle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		Handle = AssetManager.GetStreamableManager().CreateCombinedHandle({ BundleLoadHandle, RawLoadHandle });
	}
	else
	{
		Handle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}

	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnExperienceLoadComplete);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// Assets were already loaded, call the delegate now
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);

		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
			{
				OnAssetsLoadedDelegate.ExecuteIfBound();
			}));
	}

	// This set of assets gets preloaded, but we don't block the start of the experience based on it
	TSet<FPrimaryAssetId> PreloadAssetList;
	//@TODO: Determine assets to preload (but not blocking-ly)
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
}

void UGKMExperienceManagerComponent::OnExperienceLoadComplete()
{
	check(LoadState == EGKMExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);

	GKMGP_LOG(TEXT("EXPERIENCE: OnExperienceLoadComplete(CurrentExperience = %s, %s)"),
		*CurrentExperience->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));

	// find the URLs for our GameFeaturePlugins - filtering out dupes and ones that don't have a valid mapping
	GameFeaturePluginURLs.Reset();

	auto CollectGameFeaturePluginURLs = [This=this](const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString PluginURL;
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, /*out*/ PluginURL))
			{
				This->GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(false, TEXT("OnExperienceLoadComplete failed to find plugin URL from PluginName %s for experience %s - fix data, ignoring for this run"), *PluginName, *Context->GetPrimaryAssetId().ToString());
			}
		}

		// 		// Add in our extra plugin
		// 		if (!CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent.IsEmpty())
		// 		{
		// 			FString PluginURL;
		// 			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent, PluginURL))
		// 			{
		// 				GameFeaturePluginURLs.AddUnique(PluginURL);
		// 			}
		// 		}
	};

	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnable);
	/*
	for (const TObjectPtr<UGKMExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
		}
	}
	*/

	// Load and activate the features	
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = EGKMExperienceLoadState::LoadingGameFeatures;
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			UGKMExperienceManager::NotifyOfPluginActivation(PluginURL);
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(
				PluginURL, 
				FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoadComplete)
			);
		}
	}
	else
	{
		OnExperienceFullLoadCompleted();
	}
}

void UGKMExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	// decrement the number of plugins that are loading
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading == 0)
	{
		OnExperienceFullLoadCompleted();
	}
}

void UGKMExperienceManagerComponent::OnExperienceFullLoadCompleted()
{
	check(LoadState != EGKMExperienceLoadState::Loaded);

	// Insert a random delay for testing (if configured)
	if (LoadState != EGKMExperienceLoadState::LoadingChaosTestingDelay)
	{
		const float DelaySecs = GKMConsoleVariables::GetExperienceLoadDelayDuration();
		if (DelaySecs > 0.0f)
		{
			FTimerHandle DummyHandle;

			LoadState = EGKMExperienceLoadState::LoadingChaosTestingDelay;
			GetWorld()->GetTimerManager().SetTimer(DummyHandle, this, &ThisClass::OnExperienceFullLoadCompleted, DelaySecs, /*bLooping=*/ false);

			return;
		}
	}

	LoadState = EGKMExperienceLoadState::ExecutingActions;

	// Execute the actions
	FGameFeatureActivatingContext Context;

	// Only apply to our specific world context if set
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	auto ActivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action != nullptr)
			{
				//@TODO: The fact that these don't take a world are potentially problematic in client-server PIE
				// The current behavior matches systems like gameplay tags where loading and registering apply to the entire process,
				// but actually applying the results to actors is restricted to a specific world
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};
#define WITH_OLD_ACTIONS 0
#if WITH_OLD_ACTIONS
	ActivateListOfActions(CurrentExperience->Actions);
#endif

	//*
#define TMAP_ACTIONS 0
#define TARRAY_ACTIONS 1
#if TMAP_ACTIONS
	using NameSectionPair = TPair<FName, FGKMGameFeatureActionSection>;
	TArray<NameSectionPair> Sections = CurrentExperience->Sections.Array();

	Sections.Sort([](NameSectionPair const& Item) {
		return Item.Value.InsertionOrder;
	});

	TArray<UGameFeatureAction*> SectionActions;
	for (NameSectionPair const& Item : Sections) {
		for (FGKMGameFeatureAction const& Action : Item.Value.Actions) {
			SectionActions.Add(Action.Action);
		}
	}
	ActivateListOfActions(SectionActions);

#elif TARRAY_ACTIONS
	TArray<UGameFeatureAction*> SectionActions;
	for(FGKMGameFeatureActionSection const& Section: CurrentExperience->Sections) {
		for(FGKMGameFeatureAction const& Action: Section.Value) {
			SectionActions.Add(Action.Value);
		}
	}
	ActivateListOfActions(SectionActions);
#endif
#undef TARRAY_ACTIONS
#undef TMAP_ACTIONS
	//*/

	/*
	for (const TObjectPtr<UGKMExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			ActivateListOfActions(ActionSet->Actions);
		}
	}
	*/

	LoadState = EGKMExperienceLoadState::Loaded;

	// Apply any necessary scalability settings
#if !UE_SERVER
	// UGKMSettingsLocal::Get()->OnExperienceLoaded();
#endif

	// Wait for server to finish
	Barrier = 1;
	OnRep_Barrier();
}


void UGKMExperienceManagerComponent::OnRep_Barrier() {
	if (Barrier >= 1 && !bHigh) {
		GKMGP_LOG(TEXT("Trigger OnExperienceLoaded_High %d"), Barrier);

		ExperienceLoaded_High();
		OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
		OnExperienceLoaded_HighPriority.Clear();
		bHigh = true;
	}

	if (Barrier >= 2 && !bNormal) {
		GKMGP_LOG(TEXT("Trigger OnExperienceLoaded_Normal %d"), Barrier);

		ExperienceLoaded_Normal();
		OnExperienceLoaded.Broadcast(CurrentExperience);
		OnExperienceLoaded.Clear();
		bNormal = true;
	}

	if (Barrier >= 3 && !bLow) {
		GKMGP_LOG(TEXT("Trigger OnExperienceLoaded_Low %d"), Barrier);

		ExperienceLoaded_Low();
		OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
		OnExperienceLoaded_LowPriority.Clear();
		bLow = true;
	}

	if (Barrier < 3 && GetOwner()->GetNetMode() != NM_Client) {
		Barrier += 1;
		OnRep_Barrier();
	}
}

void UGKMExperienceManagerComponent::ExperienceLoaded_High() {

}
void UGKMExperienceManagerComponent::ExperienceLoaded_Normal() {

}
void UGKMExperienceManagerComponent::ExperienceLoaded_Low() {
	FGameFeatureActivatingContext Context;

	// Only apply to our specific world context if set
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	auto ActivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action != nullptr)
			{
				//@TODO: The fact that these don't take a world are potentially problematic in client-server PIE
				// The current behavior matches systems like gameplay tags where loading and registering apply to the entire process,
				// but actually applying the results to actors is restricted to a specific world
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	TArray<UGameFeatureAction*> SectionActions;
	for (FGKMGameFeatureActionSection const& Section : CurrentExperience->ExperienceLoaded_Low) {
		for (FGKMGameFeatureAction const& Action : Section.Value) {
			SectionActions.Add(Action.Value);
		}
	}
	ActivateListOfActions(SectionActions);
}

void UGKMExperienceManagerComponent::OnActionDeactivationCompleted()
{
	check(IsInGameThread());
	++NumObservedPausers;

	if (NumObservedPausers == NumExpectedPausers)
	{
		OnAllActionsDeactivated();
	}
}

void UGKMExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentExperience);
	DOREPLIFETIME(ThisClass, Barrier);
}

void UGKMExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// deactivate any features this experience loaded
	//@TODO: This should be handled FILO as well
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		if (UGKMExperienceManager::RequestToDeactivatePlugin(PluginURL))
		{
			UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
		}
	}

	//@TODO: Ensure proper handling of a partially-loaded state too
	if (LoadState == EGKMExperienceLoadState::Loaded)
	{
		LoadState = EGKMExperienceLoadState::Deactivating;

		// Make sure we won't complete the transition prematurely if someone registers as a pauser but fires immediately
		NumExpectedPausers = INDEX_NONE;
		NumObservedPausers = 0;

		// Deactivate and unload the actions
		FGameFeatureDeactivatingContext Context(TEXT(""), [this](FStringView) { this->OnActionDeactivationCompleted(); });

		const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
		if (ExistingWorldContext)
		{
			Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
		}

		auto DeactivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
		{
			for (UGameFeatureAction* Action : ActionList)
			{
				if (Action)
				{
					Action->OnGameFeatureDeactivating(Context);
					Action->OnGameFeatureUnregistering();
				}
			}
		};
#if WITH_OLD_ACTIONS
		DeactivateListOfActions(CurrentExperience->Actions);
#endif
#undef WITH_OLD_ACTIONS

		TArray<UGameFeatureAction*> SectionActions;
		for (FGKMGameFeatureActionSection const& Section : CurrentExperience->Sections) {
			for (FGKMGameFeatureAction const& Action : Section.Value) {
				SectionActions.Add(Action.Value);
			}
		}
		DeactivateListOfActions(SectionActions);

		/*
		for (const TObjectPtr<UGKMExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
		{
			if (ActionSet != nullptr)
			{
				DeactivateListOfActions(ActionSet->Actions);
			}
		}
		*/

		NumExpectedPausers = Context.GetNumPausers();

		if (NumExpectedPausers > 0)
		{
			GKMGP_ERROR(TEXT("Actions that have asynchronous deactivation aren't fully supported yet in GKM experiences"));
		}

		if (NumExpectedPausers == NumObservedPausers)
		{
			OnAllActionsDeactivated();
		}
	}
}

bool UGKMExperienceManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (LoadState != EGKMExperienceLoadState::Loaded)
	{
		OutReason = TEXT("Experience still loading");
		return true;
	}
	else
	{
		return false;
	}
}

void UGKMExperienceManagerComponent::OnAllActionsDeactivated()
{
	//@TODO: We actually only deactivated and didn't fully unload...
	LoadState = EGKMExperienceLoadState::Unloaded;
	CurrentExperience = nullptr;
	//@TODO:	GEngine->ForceGarbageCollection(true);
}


FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}
