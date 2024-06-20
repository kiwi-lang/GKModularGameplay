GKModularGameplay
=================



Condensed version of Modular Gameplay fron Lyra

1. GameMode
   * Deduce the experience to load  
      * forward the experience to the GameState::UGKMExperienceManagerComponent
   * Player Spawning is delayed until the experience is loaded

2. GameState
   * UGKMExperienceManagerComponent
      * Executes UGameFeatureAction
         * Register GameplayFecture action that will augment   
            * PlayerController
            * GameState
            * Character 
            * more
      * UGKMExperienceManagerComponent::OnExperienceLoaded.Broadcast()

3. OnExperienceLoaded
   * AGameMode::PlayerCanRestart
      * AGameMode::ControllerCanRestart
         * ULyraPlayerSpawningManagerComponent::ControllerCanRestart
   * AGameMode::RestartPlayer
      * AGameMode::FindPlayerStart
         * AGameMode::ShouldSpawnAtStartSpot
         * AGameMode::ChoosePlayerStart
            * ULyraPlayerSpawningManagerComponent::ChoosePlayerStart
      * AGameMode::RestartPlayerAtPlayerStart
         * AGameMode::FinishRestartPlayer
            * ULyraPlayerSpawningManagerComponent::FinishRestartPlayer


With GKLobby
============

* Match Join
   * Add GKChat to GameState
   * Add GKLobbyComponent to GameState
   * Push Lobby UI
   * Push Chat UI
   * Player choose their teams/heroes

* Match Lock
   * Pop Lobby
   * Teams get assigned
   * Game components gets added to Players
      * Fog Of War
      * Minimap
      * AbilitySystem
   * Abilities are added to Players
   * Push HUD
   * Players are moved to their spawn location

* Match Start
   * 

* Player Dies
   * 