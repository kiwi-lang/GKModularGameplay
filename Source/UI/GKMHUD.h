// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/HUD.h"

// Generated
#include "GKMHUD.generated.h"

namespace EEndPlayReason { enum Type : int; }

class AActor;
class UObject;

/**
 * AGKMHUD: this class is missing from ModularGameplayActors
 *
 *  Note that you typically do not need to extend or modify this class, instead you would
 *  use an "Add Widget" action in your experience to add a HUD layout and widgets to it
 * 
 *  This class exists primarily for debug rendering
 */
UCLASS(Config = Game)
class GKMODULARGAMEPLAY_API AGKMHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGKMHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	//~AActor interface
	virtual void PreInitializeComponents() override;	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AHUD interface
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
	//~End of AHUD interface
};
