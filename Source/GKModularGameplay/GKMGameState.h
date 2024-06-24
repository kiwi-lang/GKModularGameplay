// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

// GK Modular Gameplay

// Unreal Engine
// #include "AbilitySystemInterface.h"
#include "ModularGameState.h"

// Generated
#include "GKMGameState.generated.h"

struct FLyraVerbMessage;

class APlayerState;
class UAbilitySystemComponent;
class UAbilitySystemComponent;
class UGKMExperienceManagerComponent;
class UObject;
struct FFrame;

/**
 * GKMGameState
 *
 *	The base game state class used by this project.
 */
UCLASS(Config = Game)
class GKMODULARGAMEPLAY_API AGKMGameState : public AModularGameStateBase
{
	GENERATED_BODY()
public:
    AGKMGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
	// Handles loading and managing the current gameplay experience
    UPROPERTY()
	TObjectPtr<UGKMExperienceManagerComponent> ExperienceManagerComponent;

    // The ability system component subobject for game-wide things (primarily gameplay cues)
	// UPROPERTY(VisibleAnywhere, Category = "Lyra|GameState")
	// TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
