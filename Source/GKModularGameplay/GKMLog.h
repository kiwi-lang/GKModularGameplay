// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

#include "Stats/Stats.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGKMGP, Log, All);

#define GKMGP_FATAL(Format, ...)   UE_LOG(LogGKMGP, Fatal, Format, ##__VA_ARGS__)
#define GKMGP_ERROR(Format, ...)   UE_LOG(LogGKMGP, Error, Format, ##__VA_ARGS__)
#define GKMGP_WARNING(Format, ...) UE_LOG(LogGKMGP, Warning, Format, ##__VA_ARGS__)
#define GKMGP_DISPLAY(Format, ...) UE_LOG(LogGKMGP, Display, Format, ##__VA_ARGS__)
#define GKMGP_LOG(Format, ...)     UE_LOG(LogGKMGP, Log, Format, ##__VA_ARGS__)
#define GKMGP_VERBOSE(Format, ...) UE_LOG(LogGKMGP, Verbose, Format, ##__VA_ARGS__)
#define GKMGP_VERYVERBOSE(Format, ...) UE_LOG(LogGKMGP, VeryVerbose, Format, ##__VA_ARGS__)

DECLARE_STATS_GROUP(TEXT("GKMGP"), STATGROUP_GKMGP, STATCAT_Advanced);

FString GetClientServerContextString(UObject* ContextObject);