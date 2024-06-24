// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"

#include "GKMTaggedWidget.generated.h"

class UObject;

/**
 * An widget in a layout that has been tagged (can be hidden or shown via tags on the owning player)
 */
UCLASS(Abstract, Blueprintable)
class UGKMTaggedWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UGKMTaggedWidget(const FObjectInitializer& ObjectInitializer);

	//~UWidget interface
	virtual void SetVisibility(ESlateVisibility InVisibility) override;
	//~End of UWidget interface

	//~UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~End of UUserWidget interface

	UFUNCTION(BlueprintCallable, Category="Widget")
	static void ShowWidgetWithTag(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag HiddenTag);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	static void HideWidgetWithTag(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag HiddenTag);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	static void ToggleWidgetVisibility(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag HiddenTag);


protected:

	//! if true the widget will be visible at first, else not
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	bool bInitiallyVisible;

	/** If the owning player has any of these tags, this widget will be hidden (using HiddenVisibility) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FGameplayTagContainer HiddenByTags;

	/** The visibility to use when this widget is shown (not hidden by gameplay tags). */
	UPROPERTY(EditAnywhere, Category = "HUD")
	ESlateVisibility ShownVisibility = ESlateVisibility::Visible;

	/** The visibility to use when this widget is hidden by gameplay tags. */
	UPROPERTY(EditAnywhere, Category = "HUD")
	ESlateVisibility HiddenVisibility = ESlateVisibility::Collapsed;

	/** Do we want to be visible (ignoring tags)? */
	bool bWantsToBeVisible = true;

	bool HasHiddenTags();

private:
	void OnWatchedTagsChanged(const FGameplayTag Tag, int32 Count);

	TArray<FDelegateHandle> DelegateHandles;

	UPROPERTY(Transient)
	class UAbilitySystemComponent* AbilitySystemComponent;
};
