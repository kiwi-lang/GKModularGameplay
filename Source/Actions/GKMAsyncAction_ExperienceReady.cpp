// Copyright Epic Games, Inc. All Rights Reserved.

// Include
#include "GKMAsyncAction_ExperienceReady.h"

//
#include "GKMExperienceManagerComponent.h"

// Engine
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GKMAsyncAction_ExperienceReady)

UGKMAsyncAction_ExperienceReady::UGKMAsyncAction_ExperienceReady(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGKMAsyncAction_ExperienceReady* UGKMAsyncAction_ExperienceReady::WaitForExperienceReady(UObject* InWorldContextObject)
{
	UGKMAsyncAction_ExperienceReady* Action = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		Action = NewObject<UGKMAsyncAction_ExperienceReady>();
		Action->WorldPtr = World;
		Action->RegisterWithGameInstance(World);
	}

	return Action;
}

void UGKMAsyncAction_ExperienceReady::Activate()
{
	if (UWorld* World = WorldPtr.Get())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			Step2_ListenToExperienceLoading(GameState);
		}
		else
		{
			World->GameStateSetEvent.AddUObject(this, &ThisClass::Step1_HandleGameStateSet);
		}
	}
	else
	{
		// No world so we'll never finish naturally
		SetReadyToDestroy();
	}
}

void UGKMAsyncAction_ExperienceReady::Step1_HandleGameStateSet(AGameStateBase* GameState)
{
	if (UWorld* World = WorldPtr.Get())
	{
		World->GameStateSetEvent.RemoveAll(this);
	}

	Step2_ListenToExperienceLoading(GameState);
}

void UGKMAsyncAction_ExperienceReady::Step2_ListenToExperienceLoading(AGameStateBase* GameState)
{
	check(GameState);
	UGKMExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UGKMExperienceManagerComponent>();
	check(ExperienceComponent);

	if (ExperienceComponent->IsExperienceLoaded())
	{
		UWorld* World = GameState->GetWorld();
		check(World);

		// The experience happened to be already loaded, but still delay a frame to
		// make sure people don't write stuff that relies on this always being true
		//@TODO: Consider not delaying for dynamically spawned stuff / any time after the loading screen has dropped?
		//@TODO: Maybe just inject a random 0-1s delay in the experience load itself?
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::Step4_BroadcastReady));
	}
	else
	{
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(
			FGKOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::Step3_HandleExperienceLoaded)
		);
	}
}

void UGKMAsyncAction_ExperienceReady::Step3_HandleExperienceLoaded(const UGKMExperienceDefinition* CurrentExperience)
{
	Step4_BroadcastReady();
}

void UGKMAsyncAction_ExperienceReady::Step4_BroadcastReady()
{
	OnReady.Broadcast();

	SetReadyToDestroy();
}

