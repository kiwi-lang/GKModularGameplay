// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Stats/Stats.h"


class FGKModularGameplayEdModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
