// Copyright 2024 Mischievous Game, Inc. All Rights Reserved.

#pragma once

#include "ModularGameMode.h"

// Generated
#include "GKMGameMode.generated.h"

class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class UGKMExperienceDefinition;
// class ULyraPawnData;
class UObject;
struct FFrame;
struct FPrimaryAssetId;
enum class ECommonSessionOnlineMode : uint8;

/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non seamless travel
 *
 * This is called after the player has finished initialization
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FGKMOnGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

/**
 * GKMGameMode
 *
 *	The base game mode class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class GKMODULARGAMEPLAY_API AGKMGameMode : public AModularGameModeBase
{
    GENERATED_BODY()

public:
    AGKMGameMode(const FObjectInitializer& ObjectInitializer);

    //~AGameModeBase interface
    // Register OnExperienceLoaded delegate
    virtual void InitGameState() override;

    // Fetch the experience to load
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

    // virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
    // virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
    // virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
    // virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
    // virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    // virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
    // virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
    // virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
    // virtual void GenericPlayerInitialization(AController* NewPlayer) override;
    // virtual void FailedToRestartPlayer(AController* NewPlayer) override;
    //~End of AGameModeBase interface

    FGKMOnGameModePlayerInitialized OnGameModePlayerInitialized;

    bool IsExperienceLoaded() const;

protected:
    // 1
    void HandleMatchAssignmentIfNotExpectingOne();
    // 2
    void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);
    // 3
    void OnExperienceLoaded(const UGKMExperienceDefinition* CurrentExperience);

    void FetchExperienceFromURL(FPrimaryAssetId& ExperienceId, FString& ExperienceIdSource);
    void FetchExperienceFromCommandLine(FPrimaryAssetId& ExperienceId, FString& ExperienceIdSource);
    void FetchExperienceFromWorldSettings(FPrimaryAssetId& ExperienceId, FString& ExperienceIdSource);


    // Delegated to GameStateComponent
    //    Use the UGKMPlayerSpawningManagerComponent to control this!
    void    FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override final;
    AActor* ChoosePlayerStart_Implementation(AController* Player)  override final;
    bool    ControllerCanRestart(AController* Controller);
    bool    PlayerCanRestart_Implementation(APlayerController* Player)  override final
    {
        return ControllerCanRestart(Player);
    }

    // 
    AActor* FindPlayerStart_Implementation(class AController* Player, const FString& IncomingName = TEXT("")) override final {
        return Super::FindPlayerStart_Implementation(Player, IncomingName);
    }

    bool ShouldSpawnAtStartSpot(AController* Player) override final
    {
        // We never want to use the start spot, always use the spawn management component.
        return false;
    };

    bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override final
    {
        // Do nothing, we'll wait until PostLogin when we try to spawn the player for real.
        // Doing anything right now is no good, systems like team assignment haven't even occurred yet.
        return true;
    }

    void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

public:
    virtual bool TryDedicatedServerLogin();

    UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<UGKMExperienceDefinition> DefaultGameplayExperience;

    // UPROPERTY(EditAnywhere, BlueprintReadOnly)
    // FName ExperienceName;

    // UPROPERTY(EditAnywhere, BlueprintReadOnly)
    //FPrimaryAssetId DefaultExperienceId;
};
