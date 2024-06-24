// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

#include "Stats/Stats.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGKModularGameplayEd, Log, All);

#define GKMED_FATAL(Format, ...)   UE_LOG(LogGKModularGameplayEd, Fatal, Format, ##__VA_ARGS__)
#define GKMED_ERROR(Format, ...)   UE_LOG(LogGKModularGameplayEd, Error, Format, ##__VA_ARGS__)
#define GKMED_WARNING(Format, ...) UE_LOG(LogGKModularGameplayEd, Warning, Format, ##__VA_ARGS__)
#define GKMED_DISPLAY(Format, ...) UE_LOG(LogGKModularGameplayEd, Display, Format, ##__VA_ARGS__)
#define GKMED_LOG(Format, ...)     UE_LOG(LogGKModularGameplayEd, Log, Format, ##__VA_ARGS__)
#define GKMED_VERBOSE(Format, ...) UE_LOG(LogGKModularGameplayEd, Verbose, Format, ##__VA_ARGS__)
#define GKMED_VERYVERBOSE(Format, ...) UE_LOG(LogGKModularGameplayEd, VeryVerbose, Format, ##__VA_ARGS__)

DECLARE_STATS_GROUP(TEXT("GKModularGameplayEd"), STATGROUP_GKModularGameplayEd, STATCAT_Advanced);
