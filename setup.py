

# Add HUD class


# Add Loading Screen DefaultGame.ini
[/Script/CommonLoadingScreen.CommonLoadingScreenSettings]
LoadingScreenWidget=/GKModularGameplay/GKUI_LoadingScreen.GKUI_LoadingScreen_C
ForceTickLoadingScreenEvenInEditor=False


# Add Asset Manager DefaultGame.ini
[/Script/Engine.AssetManagerSettings]
+PrimaryAssetTypesToScan=(PrimaryAssetType="GKMExperienceDefinition",AssetBaseClass="/Script/GKModularGameplay.GKMExperienceDefinition",bHasBlueprintClasses=True,bIsEditorOnly=False,Directories=((Path="/Game/Levels/")),SpecificAssets=("/Game/Levels/MOBA_Experience.MOBA_Experience"),Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))


# Add new viewport  DefaultEngine.ini
[/Script/Engine.Engine]
GameViewportClientClassName=/Script/GKModularGameplay.GKMGameViewportClient
