// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GKMActivatableWidget.h"

#include "GKMHUDLayout.generated.h"

class UCommonActivatableWidget;
class UObject;


/**
 * UGKMHUDLayout
 *
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "GKM HUD Layout", Category = "GKM|HUD"))
class UGKMHUDLayout : public UGKMActivatableWidget
{
	GENERATED_BODY()

public:

	UGKMHUDLayout(const FObjectInitializer& ObjectInitializer);

	void NativeOnInitialized() override;

protected:
	void HandleEscapeAction();

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
};
