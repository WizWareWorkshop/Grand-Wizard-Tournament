#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// API Macro Definition
#if WITH_EDITOR
    #define GRIMOIREPLUGIN_API DLLEXPORT
#else
    #if defined(GRIMOIREPLUGIN_API)
        #define GRIMOIREPLUGIN_API DLLEXPORT
    #else
        #define GRIMOIREPLUGIN_API DLLIMPORT
    #endif
#endif

class GRIMOIREPLUGIN_API FGrimoirePlugin : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static inline FGrimoirePlugin& Get()
    {
        return FModuleManager::LoadModuleChecked<FGrimoirePlugin>("Grimoire");
    }

    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("Grimoire");
    }
};