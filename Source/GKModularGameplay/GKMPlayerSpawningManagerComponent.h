// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Unreal Engine
#include "Components/GameStateComponent.h"
#include "UObject/Interface.h"

// Generated
#include "GKMPlayerSpawningManagerComponent.generated.h"

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class AGKMPlayerStart;
class AActor;

USTRUCT()
struct FGKCachedStartSpot {

	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	TObjectPtr<APlayerStart> StartSpot;
	
	UPROPERTY()
	TObjectPtr<AController> Controller;

	bool operator== (APlayerStart const* OtherStartSpot) const {
		return StartSpot == OtherStartSpot;
	}
};



/** Interface for actors which can be associated with teams */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UGKMPlayerSpawnerInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class GKMODULARGAMEPLAY_API IGKMPlayerSpawnerInterface
{
	GENERATED_IINTERFACE_BODY()
};


/**
 * Retrieve all the PlayerStart actors from the map.
 * Assign PlayerStart to players
 * 
 * This components replace 3 calls from the game mode to override the StartLocation logic.
 */
UCLASS()
class GKMODULARGAMEPLAY_API UGKMPlayerSpawningManagerComponent : public UGameStateComponent, public IGKMPlayerSpawnerInterface
{
	GENERATED_BODY()

public:
	UGKMPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	/** UActorComponent */
	virtual void InitializeComponent() override;
	/** ~UActorComponent */

protected:
	// Helper to select the first un occupied player start
	UFUNCTION(BlueprintCallable, Category = "Player|Start")
	APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<APlayerStart*>& FoundStartPoints) const;
	
	//! Return all unclaimed starting spot
	UFUNCTION(BlueprintCallable, Category = "Player|Start")
	void GetFreeStartingSpot(TArray<APlayerStart*>& StarterPoints);

	//! Retrieve all the starting spot even the one that are claimed
	UFUNCTION(BlueprintCallable, Category = "Player|Start")
	void GetAllStartingSpot(TArray<APlayerStart*>& StarterPoints);

	//! Claim a start spot so no other controller can use it
	UFUNCTION(BlueprintCallable, Category = "Player|Start")
	void Claim(AController* Controller, APlayerStart* PlayerStart);

	/** We proxy these calls from AGKMGameMode, to this component so that each experience can more easily customize the respawn system they want. */

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Player|Start")
	AActor* ChoosePlayerStart(AController* Player);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player|Start")
	bool ControllerCanRestart(AController* Player);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player|Start")
	void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);

	//! Returns true if the controller is only a spectator
	UFUNCTION(BlueprintPure, Category = "Player|Start")
	bool IsSpectator(AController* Player);

	//! Select a start point at random without claiming it
	UFUNCTION(BlueprintPure, Category = "Player|Start")
	AActor* ChoosePlayerStartForSpectator(AController* Player);

	TArray<FGKCachedStartSpot> const& GetPlayerStarts() {
		return CachedPlayerStarts;
	}

private:
	AActor* ChoosePlayerStart_Native(AController* Player);

	friend class AGKMGameMode;
	/** ~AGKMGameMode */

	UPROPERTY(Transient)
	TArray<FGKCachedStartSpot> CachedPlayerStarts;

private:
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};
