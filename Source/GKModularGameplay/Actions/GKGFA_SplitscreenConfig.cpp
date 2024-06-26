// Copyright Epic Games, Inc. All Rights Reserved.


// Include
#include "GKGFA_SplitscreenConfig.h"

//
#include "GKGFA_WorldActionBase.h"

// Unreal Engine
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GKGFA_SplitscreenConfig)

#define LOCTEXT_NAMESPACE "GKGameFeatures"

//////////////////////////////////////////////////////////////////////
// UGKGFA_SplitscreenConfig

TMap<FObjectKey, int32> UGKGFA_SplitscreenConfig::GlobalDisableVotes;

void UGKGFA_SplitscreenConfig::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	for (int32 i = LocalDisableVotes.Num() - 1; i >= 0; i-- )
	{
		FObjectKey ViewportKey = LocalDisableVotes[i];
		UGameViewportClient* GVP = Cast<UGameViewportClient>(ViewportKey.ResolveObjectPtr());
		const FWorldContext* WorldContext = GEngine->GetWorldContextFromGameViewport(GVP);
		if (GVP && WorldContext)
		{
			if (!Context.ShouldApplyToWorldContext(*WorldContext))
			{
				// Wrong context so ignore it, dead objects count as part of this context
				continue;
			}
		}

		int32& VoteCount = GlobalDisableVotes[ViewportKey];
		if (VoteCount <= 1)
		{
			GlobalDisableVotes.Remove(ViewportKey);

			if (GVP && WorldContext)
			{
				GVP->SetForceDisableSplitscreen(false);
			}
		}
		else
		{
			--VoteCount;
		}
		LocalDisableVotes.RemoveAt(i);
	}
}

void UGKGFA_SplitscreenConfig::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	if (bDisableSplitscreen)
	{
		if (UGameInstance* GameInstance = WorldContext.OwningGameInstance)
		{
			if (UGameViewportClient* VC = GameInstance->GetGameViewportClient())
			{
				FObjectKey ViewportKey(VC);

				LocalDisableVotes.Add(ViewportKey);

				int32& VoteCount = GlobalDisableVotes.FindOrAdd(ViewportKey);
				VoteCount++;
				if (VoteCount == 1)
				{
					VC->SetForceDisableSplitscreen(true);
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

