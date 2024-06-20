// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//
#include "CommonLocalPlayer.h"
#include "GenericTeamAgentInterface.h"

// Generated
#include "GKMLocalPlayer.generated.h"

struct FGenericTeamId;

class APlayerController;
class UInputMappingContext;
class UGKMSettingsLocal;
class UGKMSettingsShared;
class UObject;
class UWorld;
struct FFrame;
struct FSwapAudioOutputResult;

/**
 * UGKMLocalPlayer
 */
UCLASS()
class GKMODULARGAMEPLAY_API UGKMLocalPlayer : public UCommonLocalPlayer, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	UGKMLocalPlayer();

	//~UObject interface
	virtual void PostInitProperties() override;
	//~End of UObject interface

	//~UPlayer interface
	virtual void SwitchController(class APlayerController* PC) override;
	//~End of UPlayer interface

	//~ULocalPlayer interface
	virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;
	virtual void InitOnlineSession() override;
	//~End of ULocalPlayer interface

	//~IGKMTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	// virtual FOnGKMTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IGKMTeamAgentInterface interface

	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	// UFUNCTION()
	// UGKMSettingsLocal* GetLocalSettings() const;

	/** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	// UFUNCTION()
	// UGKMSettingsShared* GetSharedSettings() const;

	/** Starts an async request to load the shared settings, this will call OnSharedSettingsLoaded after loading or creating new ones */
	// void LoadSharedSettingsFromDisk(bool bForceLoad = false);

protected:
	// void OnSharedSettingsLoaded(UGKMSettingsShared* LoadedOrCreatedSettings);

	void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);
	
	//UFUNCTION()
	//void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

	void OnPlayerControllerChanged(APlayerController* NewController);

	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	// UPROPERTY(Transient)
	//mutable TObjectPtr<UGKMSettingsShared> SharedSettings;

	FUniqueNetIdRepl NetIdForSharedSettings;

	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

	// UPROPERTY()
	//FOnGKMTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
};
