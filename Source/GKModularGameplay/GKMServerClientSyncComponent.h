// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Unreal Engine
#include "Components/GameStateComponent.h"
#include "UObject/Interface.h"

// Generated
#include "GKMServerClientSyncComponent.generated.h"

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class AGKMPlayerStart;
class AActor;


/**
 * Retrieve all the PlayerStart actors from the map.
 * Assign PlayerStart to players
 *
 * This components replace 3 calls from the game mode to override the StartLocation logic.
 */
UCLASS()
class GKMODULARGAMEPLAY_API UGKMServerClientSyncComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	// UGKMServerClientSyncComponent(const FObjectInitializer& ObjectInitializer);

	/** UActorComponent */
	// virtual void InitializeComponent() override;
	/** ~UActorComponent */


    bool bServerReady;
};
