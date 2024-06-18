// Copyright Epic Games, Inc. All Rights Reserved.

// Include
#include "GKMPlayerSpawningManagerComponent.h"

//
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"
// #include "GKMPlayerStart.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GKMPlayerSpawningManagerComponent)

DEFINE_LOG_CATEGORY_STATIC(LogPlayerSpawning, Log, All);



UGKMPlayerSpawnerInterface::UGKMPlayerSpawnerInterface(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer)
{}

UGKMPlayerSpawningManagerComponent::UGKMPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;

	// Lyra made this tick !?
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGKMPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	UWorld* World = GetWorld();
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (APlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(FGKCachedStartSpot{
				PlayerStart,
				nullptr
			});
		}
	}
}

void UGKMPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(FGKCachedStartSpot{
					PlayerStart,
					nullptr
				});
			}
		}
	}
}

void UGKMPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	if (APlayerStart* PlayerStart = Cast<APlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(FGKCachedStartSpot{
			PlayerStart,
			nullptr
		});
	}
}

// AGKMGameMode Proxied Calls - Need to handle when someone chooses
// to restart a player the normal way in the engine.
//======================================================================


void UGKMPlayerSpawningManagerComponent::GetFreeStartingSpot(TArray<APlayerStart*>& StarterPoints) {
	for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
	{
		FGKCachedStartSpot& StartSpot = *StartIt;

		if (APlayerStart* Start = StartSpot.StartSpot.Get())
		{
			if (StartSpot.Controller == nullptr) {
				StarterPoints.Add(Start);
			}
		}
		else
		{
			StartIt.RemoveCurrent();
		}
	}
}

void UGKMPlayerSpawningManagerComponent::GetAllStartingSpot(TArray<APlayerStart*>& StarterPoints) {
	for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
	{
		FGKCachedStartSpot& StartSpot = *StartIt;

		if (APlayerStart* Start = StartSpot.StartSpot.Get())
		{
			StarterPoints.Add(Start);
		}
		else
		{
			StartIt.RemoveCurrent();
		}
	}
}


bool UGKMPlayerSpawningManagerComponent::IsSpectator(AController* Player) 
{
	if (APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
	{
		// start dedicated spectators at any random starting location, but they do not claim it
		if (PlayerState->IsOnlyASpectator())
		{
			return true;
		}
	}
	return false;
}



AActor* UGKMPlayerSpawningManagerComponent::ChoosePlayerStartForSpectator(AController* Player) 
{
	TArray<APlayerStart*> StarterPoints;
	GetAllStartingSpot(StarterPoints);

	if (!StarterPoints.IsEmpty())
	{
		return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)];
	}

	return nullptr;
}

UE_DISABLE_OPTIMIZATION
AActor* UGKMPlayerSpawningManagerComponent::ChoosePlayerStart_Native(AController* Player) {
	//
	// There is so weird thigns happening here
	// the code stops at the second if and never call the last `ChoosePlayerStart`
	// unless UE_DISABLE_OPTIMIZATION
	//
	if (Player)
	{
		#if WITH_EDITOR
		APlayerStart* PlayerStart = FindPlayFromHereStart(Player);
		if (PlayerStart)
		{
			return PlayerStart;
		}
		#endif

		if (IsSpectator(Player))
		{
			return ChoosePlayerStartForSpectator(Player);
		}

		return ChoosePlayerStart(Player);
	}
	return nullptr;
}
UE_ENABLE_OPTIMIZATION

AActor* UGKMPlayerSpawningManagerComponent::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> StarterPoints;
	GetFreeStartingSpot(StarterPoints);
	return  GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints);
}

#if WITH_EDITOR
APlayerStart* UGKMPlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
	// Only 'Play From Here' for a player controller, bots etc. should all spawn from normal spawn points.
	if (Player->IsA<APlayerController>())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				if (APlayerStart* PlayerStart = *It)
				{
					if (PlayerStart->IsA<APlayerStartPIE>())
					{
						// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
						return PlayerStart;
					}
				}
			}
		}
	}

	return nullptr;
}
#endif

bool UGKMPlayerSpawningManagerComponent::ControllerCanRestart_Implementation(AController* Player)
{
	return true;
}

void UGKMPlayerSpawningManagerComponent::FinishRestartPlayer_Implementation(AController* NewPlayer, const FRotator& StartRotation)
{
}

//================================================================

enum class EGKMPlayerStartLocationOccupancy
{
	Empty,
	Partial,
	Full
};

EGKMPlayerStartLocationOccupancy GetLocationOccupancy(APlayerStart const* PlayerStart, AController* const ControllerPawnToFit)
{
	UWorld* const World = PlayerStart->GetWorld();
	if (PlayerStart->HasAuthority() && World)
	{
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector ActorLocation = PlayerStart->GetActorLocation();
			const FRotator ActorRotation = PlayerStart->GetActorRotation();

			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation, nullptr))
			{
				return EGKMPlayerStartLocationOccupancy::Empty;
			}
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				return EGKMPlayerStartLocationOccupancy::Partial;
			}
		}
	}

	return EGKMPlayerStartLocationOccupancy::Full;
}

void UGKMPlayerSpawningManagerComponent::Claim(AController* Controller, APlayerStart* PlayerStart) {
	Controller->StartSpot = PlayerStart;

	for (FGKCachedStartSpot& Start: CachedPlayerStarts)
	{
		if (Start.StartSpot.Get() == PlayerStart) {
			Start.Controller = Controller;
		}
	}
}

APlayerStart* UGKMPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<APlayerStart*>& StartPoints) const
{
	if (Controller)
	{
		TArray<APlayerStart*> UnOccupiedStartPoints;
		TArray<APlayerStart*> OccupiedStartPoints;

		for (APlayerStart* StartPoint : StartPoints)
		{
			EGKMPlayerStartLocationOccupancy State = GetLocationOccupancy(StartPoint, Controller);

			switch (State)
			{
				case EGKMPlayerStartLocationOccupancy::Empty:
					UnOccupiedStartPoints.Add(StartPoint);
					break;
				case EGKMPlayerStartLocationOccupancy::Partial:
					OccupiedStartPoints.Add(StartPoint);
					break;

			}
		}

		if (UnOccupiedStartPoints.Num() > 0)
		{
			return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}

	return nullptr;
}

