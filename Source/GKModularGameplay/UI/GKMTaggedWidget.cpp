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

void UGKMTaggedWidget::ShowWidgetWithTag(UAbilitySystemComponent* ASC, FGameplayTag Tag) {
	if (ASC) {
		ASC->RemoveLooseGameplayTag(Tag);
		return;
	}
	GKMGP_WARNING(TEXT("UGKMTaggedWidget: UAbilitySystemComponent is null"));
}

void UGKMTaggedWidget::HideWidgetWithTag(UAbilitySystemComponent* ASC, FGameplayTag Tag) {
	if (ASC) {
		ASC->AddLooseGameplayTag(Tag);
		return;
	}
	GKMGP_WARNING(TEXT("UGKMTaggedWidget: UAbilitySystemComponent is null"));
}

void UGKMTaggedWidget::ToggleWidgetVisibility(class UAbilitySystemComponent* ASC, FGameplayTag HiddenTag) {
	if (ASC) {
		if (ASC->HasMatchingGameplayTag(HiddenTag)) {
			ShowWidgetWithTag(ASC, HiddenTag);
		} else {
			HideWidgetWithTag(ASC, HiddenTag);
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
			for(FGameplayTag Tag: HiddenByTags) {
				DelegateHandles.Add(AbilitySystemComponent->RegisterGameplayTagEvent(
					Tag,
					EGameplayTagEventType::NewOrRemoved
				).AddUObject(this, &UGKMTaggedWidget::OnWatchedTagsChanged));
			}
		}

		// Set our initial visibility value (checking the tags, etc...)
		SetVisibility(GetVisibility());

		if (HiddenByTags.Num() == 1) {
			FGameplayTag Tag = HiddenByTags.GetByIndex(0);

			if (bInitiallyVisible && HasHiddenTags()) {
				UGKMTaggedWidget::ToggleWidgetVisibility(AbilitySystemComponent, Tag);
			}

			if (!bInitiallyVisible && !HasHiddenTags()) {
				UGKMTaggedWidget::ToggleWidgetVisibility(AbilitySystemComponent, Tag);
			}
		}
	}
}

void UGKMTaggedWidget::NativeDestruct()
{
	if (!IsDesignTime())
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

	const bool bHasHiddenTags = HasHiddenTags();

	// Actually apply the visibility
	const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
	if (GetVisibility() != DesiredVisibility)
	{
		Super::SetVisibility(DesiredVisibility);
	}
}

bool UGKMTaggedWidget::HasHiddenTags() {
	if (AbilitySystemComponent) {
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(HiddenByTags);
	}
	return false;
}

void UGKMTaggedWidget::OnWatchedTagsChanged(FGameplayTag const Tag, int32 Count)
{
	const bool bHasHiddenTags = HasHiddenTags();

	// Actually apply the visibility
	const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
	if (GetVisibility() != DesiredVisibility)
	{
		Super::SetVisibility(DesiredVisibility);
	}
}

