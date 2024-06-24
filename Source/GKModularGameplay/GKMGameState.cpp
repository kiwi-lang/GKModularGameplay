// Include
#include "GKMGameState.h"

// GK Modular Gameplay
#include "GKMExperienceManagerComponent.h"


AGKMGameState::AGKMGameState(const FObjectInitializer& ObjectInitializer):
    Super(ObjectInitializer)
{
    ExperienceManagerComponent = CreateDefaultSubobject<UGKMExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
}