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
	static void ShowWidgetWitHiddenTag(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag HiddenTag);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	static void HideWidgetWithHiddenTag(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag HiddenTag);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	static void ShowWidgetWitShownTag(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag ShownTag);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	static void HideWidgetWithShownTag(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag ShownTag);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	static void ToggleWidgetVisibilityWithHiddenTag(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag HiddenTag);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	static void ToggleWidgetVisibilityWithShownTag(class UAbilitySystemComponent* AbilitySystemComponent, FGameplayTag ShownTag);

protected:

	//! if true the widget will be visible at first, else not
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	bool bInitiallyVisible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShownByTags;

	/** If the owning player has any of these tags, this widget will be hidden (using HiddenVisibility) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD", meta = (EditCondition = "!bShownByTags"))
	FGameplayTagContainer HiddenByTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD", meta = (EditCondition = "bShownByTags"))
	FGameplayTagContainer ShownByTags;

	/** The visibility to use when this widget is shown (not hidden by gameplay tags). */
	UPROPERTY(EditAnywhere, Category = "HUD")
	ESlateVisibility ShownVisibility = ESlateVisibility::Visible;

	/** The visibility to use when this widget is hidden by gameplay tags. */
	UPROPERTY(EditAnywhere, Category = "HUD")
	ESlateVisibility HiddenVisibility = ESlateVisibility::Collapsed;

	/** Do we want to be visible (ignoring tags)? */
	bool bWantsToBeVisible = true;

	bool HasHiddenTags();

	bool HasShownTags();

private:
	void OnWatchedTagsChanged(const FGameplayTag Tag, int32 Count);

	TArray<FDelegateHandle> DelegateHandles;

	UPROPERTY(Transient)
	class UAbilitySystemComponent* AbilitySystemComponent;
};
