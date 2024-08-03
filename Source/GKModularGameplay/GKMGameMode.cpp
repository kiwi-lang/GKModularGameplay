// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKMGameMode.h"

// Game
#include "GKMLog.h"
#include "GKMWorldSettings.h"
#include "GKMExperienceManagerComponent.h"
#include "GKMAssetManager.h"
#include "GKMExperienceDefinition.h"
#include "GKMPlayerSpawningManagerComponent.h"
#include "GKMGameState.h"
#include "UI/GKMHUD.h"

// Modular
#include "ModularPlayerController.h"
#include "ModularPlayerState.h"
#include "ModularPawn.h"

// Unreal Engine
#include "Kismet/GameplayStatics.h"


AGKMGameMode::AGKMGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AGKMGameState::StaticClass();
	// GameSessionClass = AGKMGameSession::StaticClass();
	PlayerControllerClass = AModularPlayerController::StaticClass();
	// ReplaySpectatorPlayerControllerClass = AGKMReplayPlayerController::StaticClass();
	PlayerStateClass = AModularPlayerState::StaticClass();
	DefaultPawnClass = AModularPawn::StaticClass();
	HUDClass = AGKMHUD::StaticClass();

	/*
	DefaultExperienceId = FPrimaryAssetId(
		FPrimaryAssetType("GKMExperienceDefinition"),
		FName("B_LyraDefaultExperience")
	);*/
}

void AGKMGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Wait for the next frame to give time to initialize startup settings
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

void AGKMGameMode::InitGameState()
{
	Super::InitGameState();

	// Listen for the experience load to complete	
	UGKMExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UGKMExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(
        FGKOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded)
    );
}


void AGKMGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
    FPrimaryAssetId ExperienceId;
    FString ExperienceIdSource = "NOTFOUND";

    FetchExperienceFromURL(ExperienceId, ExperienceIdSource);
    FetchExperienceFromCommandLine(ExperienceId, ExperienceIdSource);
    FetchExperienceFromWorldSettings(ExperienceId, ExperienceIdSource);

	UGKMAssetManager& AssetManager = UGKMAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*out*/ Dummy))
	{
		GKMGP_ERROR(TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	// Final fallback to the default experience
	if (!ExperienceId.IsValid())
	{
		if (TryDedicatedServerLogin())
		{
			// This will start to host as a dedicated server
			return;
		}

		//@TODO: Pull this from a config setting or something
		ExperienceId = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());
        // FPrimaryAssetId(
		// 	UGKMExperienceDefinition::StaticClass()->GetFName()),
		// 	ExperienceName									//FName("B_LyraDefaultExperience")
		// );
		ExperienceIdSource = TEXT("Default");
	}

	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

void AGKMGameMode::FetchExperienceFromURL(FPrimaryAssetId& ExperienceId, FString& ExperienceIdSource) {
    if (ExperienceId.IsValid()) {
        return;
    }

	if (UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(
            FPrimaryAssetType(UGKMExperienceDefinition::StaticClass()->GetFName()), 
            FName(*ExperienceFromOptions)
        );
		ExperienceIdSource = TEXT("OptionsString");
	}
}

void AGKMGameMode::FetchExperienceFromCommandLine(FPrimaryAssetId& ExperienceId, FString& ExperienceIdSource) {
    if (ExperienceId.IsValid()) {
        return;
    }
    FString ExperienceFromCommandLine;
    if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
    {
        ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
        if (!ExperienceId.PrimaryAssetType.IsValid())
        {
            ExperienceId = FPrimaryAssetId(
                FPrimaryAssetType(UGKMExperienceDefinition::StaticClass()->GetFName()), 
                FName(*ExperienceFromCommandLine)
            );
        }
        ExperienceIdSource = TEXT("CommandLine");
    }
}

void AGKMGameMode::FetchExperienceFromWorldSettings(FPrimaryAssetId& ExperienceId, FString& ExperienceIdSource) {
    if (ExperienceId.IsValid()) {
        return;
    }

    if (AGKMWorldSettings* TypedWorldSettings = Cast<AGKMWorldSettings>(GetWorldSettings()))
    {
        ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
        ExperienceIdSource = TEXT("WorldSettings");
    }
}

void AGKMGameMode::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)  {
    if (ExperienceId.IsValid())
	{
		GKMGP_LOG(TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(), *ExperienceIdSource);

		UGKMExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UGKMExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->SetCurrentExperience(ExperienceId);
	}
	else
	{
		GKMGP_ERROR(TEXT("Failed to identify experience, loading screen will stay up forever"));
	}
}


bool AGKMGameMode::TryDedicatedServerLogin() {
	return false;
}

void AGKMGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Delay starting new players until the experience has been loaded
	// (players who log in prior to that will be started by OnExperienceLoaded)
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

void AGKMGameMode::OnExperienceLoaded(const UGKMExperienceDefinition* CurrentExperience)
{
	// Spawn any players that are already attached
	// @TODO: Here we're handling only *player* controllers, but in GetDefaultPawnClassForController_Implementation we skipped all controllers
	// GetDefaultPawnClassForController_Implementation might only be getting called for players anyways
	
	
	// Experience is loaded, restart the player
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if ((PC != nullptr) && (PC->GetPawn() == nullptr))
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}

	// Player is ready, add widget handling here
}

bool AGKMGameMode::IsExperienceLoaded() const
{
	check(GameState);
	UGKMExperienceManagerComponent* ExperienceComponent = GameState->GetComponentByClass<UGKMExperienceManagerComponent>();
	check(ExperienceComponent);
	return ExperienceComponent->IsExperienceLoaded();
}

UGKMPlayerSpawningManagerComponent* GetSpawner(AGameStateBase* GameState) {
	// UActorComponent* Component = GameState->FindComponentByInterface(UGKMPlayerSpawnerInterface::StaticClass());
	// return Cast<UGKMPlayerSpawningManagerComponent>(Component);

	return GameState->FindComponentByClass<UGKMPlayerSpawningManagerComponent>();
}


AActor* AGKMGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (UGKMPlayerSpawningManagerComponent* PlayerSpawningComponent = GetSpawner(GameState))
	{
		return PlayerSpawningComponent->ChoosePlayerStart_Native(Player);
	}
	
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AGKMGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	if (UGKMPlayerSpawningManagerComponent* PlayerSpawningComponent = GetSpawner(GameState))
	{
		PlayerSpawningComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	}

	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

bool AGKMGameMode::ControllerCanRestart(AController* Controller)
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{	
		if (!Super::PlayerCanRestart_Implementation(PC))
		{
			return false;
		}
	}
	else
	{
		// Bot version of Super::PlayerCanRestart_Implementation
		if ((Controller == nullptr) || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	if (UGKMPlayerSpawningManagerComponent* PlayerSpawningComponent = GetSpawner(GameState))
	{
		return PlayerSpawningComponent->ControllerCanRestart(Controller);
	}

	return true;
}