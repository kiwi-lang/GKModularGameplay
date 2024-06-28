// Copyright Epic Games, Inc. All Rights Reserved.


// Include
#include "GKMTaggedWidget.h"

//
#include "GKMLog.h"


// Unreal Engine
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GKMTaggedWidget)

//@TODO: The other TODOs in this file are all related to tag-based showing/hiding of widgets, see UE-142237

UGKMTaggedWidget::UGKMTaggedWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShownByTags = false;
}


class UAbilitySystemComponent* _FindAbilitySystemComponent(AActor* Actor) {
    IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(Actor);
    if (Interface) {
        return Cast<UAbilitySystemComponent>(Interface->GetAbilitySystemComponent());
    }
 
	return Actor->GetComponentByClass<UAbilitySystemComponent>();
}

class UAbilitySystemComponent* FindAbilitySystemComponent(UUserWidget* Widget) {
    if (Widget) {
        UAbilitySystemComponent* ASC = nullptr;

        APawn* Pawn = Widget->GetOwningPlayerPawn();
        if (Pawn) {
            if (UAbilitySystemComponent* EventComp = _FindAbilitySystemComponent(Pawn)) {
                ASC = EventComp;
            }
        }

        APlayerController* Controller = Widget->GetOwningPlayer();
        if (Controller) {
            if (UAbilitySystemComponent* EventComp = _FindAbilitySystemComponent(Controller)) {
                ASC = EventComp;
            }
        }

        APlayerState* PlayerState = Widget->GetOwningPlayerState();
        if (PlayerState) {
            if (UAbilitySystemComponent* EventComp = _FindAbilitySystemComponent(PlayerState)) {
                ASC = EventComp;
            }
        }

        if (ASC) {
            return ASC;
        }
    }

	GKMGP_WARNING(TEXT("UGKMTaggedWidget: Cloud not find UAbilitySystemComponent"));
    return nullptr;
}

void UGKMTaggedWidget::ShowWidgetWitHiddenTag(UAbilitySystemComponent* ASC, FGameplayTag Tag) {
	if (ASC) {
		ASC->RemoveLooseGameplayTag(Tag);
		return;
	}
	GKMGP_WARNING(TEXT("UGKMTaggedWidget: UAbilitySystemComponent is null"));
}

void UGKMTaggedWidget::HideWidgetWithHiddenTag(UAbilitySystemComponent* ASC, FGameplayTag Tag) {
	if (ASC) {
		ASC->AddLooseGameplayTag(Tag);
		return;
	}
	GKMGP_WARNING(TEXT("UGKMTaggedWidget: UAbilitySystemComponent is null"));
}


void UGKMTaggedWidget::ShowWidgetWitShownTag(UAbilitySystemComponent* ASC, FGameplayTag Tag) {
	if (ASC) {
		ASC->AddLooseGameplayTag(Tag);
		return;
	}
	GKMGP_WARNING(TEXT("UGKMTaggedWidget: UAbilitySystemComponent is null"));
}

void UGKMTaggedWidget::HideWidgetWithShownTag(UAbilitySystemComponent* ASC, FGameplayTag Tag) {
	if (ASC) {
		ASC->RemoveLooseGameplayTag(Tag);
		return;
	}
	GKMGP_WARNING(TEXT("UGKMTaggedWidget: UAbilitySystemComponent is null"));
}

void UGKMTaggedWidget::ToggleWidgetVisibilityWithHiddenTag(class UAbilitySystemComponent* ASC, FGameplayTag HiddenTag) {
	if (ASC) {
		if (ASC->HasMatchingGameplayTag(HiddenTag)) {
			ShowWidgetWitHiddenTag(ASC, HiddenTag);
		}
		else {
			HideWidgetWithHiddenTag(ASC, HiddenTag);
		}
		return;
	}
	GKMGP_WARNING(TEXT("UGKMTaggedWidget: UAbilitySystemComponent is null"));
}

void UGKMTaggedWidget::ToggleWidgetVisibilityWithShownTag(class UAbilitySystemComponent* ASC, FGameplayTag ShownTag) {
	if (ASC) {
		if (!ASC->HasMatchingGameplayTag(ShownTag)) {
			ShowWidgetWitShownTag(ASC, ShownTag);
		}
		else {
			HideWidgetWithShownTag(ASC, ShownTag);
		}
		return;
	}
	GKMGP_WARNING(TEXT("UGKMTaggedWidget: UAbilitySystemComponent is null"));
}


void UGKMTaggedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsDesignTime())
	{
		// Listen for tag changes on our hidden tags
		
		// IGameplayTagAssetInterface

		AbilitySystemComponent = FindAbilitySystemComponent(this);
		
		if (AbilitySystemComponent) {
			if (bShownByTags) {
				for (FGameplayTag Tag : ShownByTags) {
					DelegateHandles.Add(AbilitySystemComponent->RegisterGameplayTagEvent(
						Tag,
						EGameplayTagEventType::NewOrRemoved
					).AddUObject(this, &UGKMTaggedWidget::OnWatchedTagsChanged));
				}
			}
			else {
				for(FGameplayTag Tag: HiddenByTags) {
					DelegateHandles.Add(AbilitySystemComponent->RegisterGameplayTagEvent(
						Tag,
						EGameplayTagEventType::NewOrRemoved
					).AddUObject(this, &UGKMTaggedWidget::OnWatchedTagsChanged));
				}
			}

		}

		// Set our initial visibility value (checking the tags, etc...)
		SetVisibility(GetVisibility());

		if (bShownByTags) {
			if (ShownByTags.Num() == 1) {
				FGameplayTag Tag = ShownByTags.GetByIndex(0);

				if (bInitiallyVisible && !HasShownTags()) {
					UGKMTaggedWidget::ToggleWidgetVisibilityWithShownTag(AbilitySystemComponent, Tag);
				}

				if (!bInitiallyVisible && HasShownTags()) {
					UGKMTaggedWidget::ToggleWidgetVisibilityWithShownTag(AbilitySystemComponent, Tag);
				}
			}
		}
		else {
			if (HiddenByTags.Num() == 1) {
				FGameplayTag Tag = HiddenByTags.GetByIndex(0);

				if (bInitiallyVisible && HasHiddenTags()) {
					UGKMTaggedWidget::ToggleWidgetVisibilityWithHiddenTag(AbilitySystemComponent, Tag);
				}

				if (!bInitiallyVisible && !HasHiddenTags()) {
					UGKMTaggedWidget::ToggleWidgetVisibilityWithHiddenTag(AbilitySystemComponent, Tag);
				}
			}
		}
	}
}

void UGKMTaggedWidget::NativeDestruct()
{
	if (!IsDesignTime())
	{
		if (bShownByTags) {
			if (AbilitySystemComponent) {
				for (FGameplayTag Tag : ShownByTags) {
					AbilitySystemComponent->RegisterGameplayTagEvent(
						Tag,
						EGameplayTagEventType::NewOrRemoved
					).RemoveAll(this);
				}
			}
		} 
		else 
		{
			if (AbilitySystemComponent) {
				for (FGameplayTag Tag : HiddenByTags) {
					AbilitySystemComponent->RegisterGameplayTagEvent(
						Tag,
						EGameplayTagEventType::NewOrRemoved
					).RemoveAll(this);
				}
			}
		}
	}

	Super::NativeDestruct();
}

void UGKMTaggedWidget::SetVisibility(ESlateVisibility InVisibility)
{
#if WITH_EDITORONLY_DATA
	if (IsDesignTime())
	{
		Super::SetVisibility(InVisibility);
		return;
	}
#endif

	// Remember what the caller requested; even if we're currently being
	// suppressed by a tag we should respect this call when we're done
	bWantsToBeVisible = ConvertSerializedVisibilityToRuntime(InVisibility).IsVisible();
	if (bWantsToBeVisible)
	{
		ShownVisibility = InVisibility;
	}
	else
	{
		HiddenVisibility = InVisibility;
	}

	if (!bWantsToBeVisible) {
		GKMGP_LOG(TEXT("UGKMTaggedWidget: does not want to be visible"));
	}


	OnWatchedTagsChanged(FGameplayTag(), 0);
}

bool UGKMTaggedWidget::HasHiddenTags() {
	if (AbilitySystemComponent) {
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(HiddenByTags);
	}
	return false;
}

bool UGKMTaggedWidget::HasShownTags() {
	if (AbilitySystemComponent) {
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(ShownByTags);
	}
	return false;
}

void UGKMTaggedWidget::OnWatchedTagsChanged(FGameplayTag const Tag, int32 Count)
{

	if (bShownByTags) {
		const bool bHasShownByTags = HasShownTags();

		// Actually apply the visibility
		const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && bHasShownByTags) ? ShownVisibility : HiddenVisibility;
		if (GetVisibility() != DesiredVisibility)
		{
			Super::SetVisibility(DesiredVisibility);
		}
	}
	else {
		const bool bHasHiddenTags = HasHiddenTags();

		// Actually apply the visibility
		const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
		if (GetVisibility() != DesiredVisibility)
		{
			Super::SetVisibility(DesiredVisibility);
		}
	}
}

