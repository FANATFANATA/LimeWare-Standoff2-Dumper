#include "LimeWare.h"
#include <KittyMemOp.hpp>

bool LimeWare::initialize(bool inject)
{
    if (inject)
    {
        g().log->info("Injecting LimeWare-Dumper library...");
        return injection->start();
    }

    if (KittyMemoryEx::getProcessName(getpid()) != "com.axlebolt.standoff2")
    {
        return false;
    }

    g().log->info("Initializing Memory manager...");
    if (!g().memory->initialize())
    {
        g().log->error("Memory manager couldn't be initialized!");
        return false;
    }

    g().log->info("Initializing Il2cpp manager...");
    if (!g().il2cpp->initialize())
    {
        g().log->error("Il2cpp manager couldn't be initialized!");
        return false;
    }

    g().log->info("Initializing Dumper manager...");
    if (!g().dumper->initialize())
    {
        g().log->error("Dumper manager couldn't be initialized!");
        return false;
    }

    return true;
}

LimeWare &g()
{
    static LimeWare instance = LimeWare(std::make_unique<Log>(), std::make_unique<Injection>(std::make_unique<KittyMemoryMgr>()),
                                        std::make_unique<Dumper>(), std::make_unique<Memory>(), std::make_unique<Il2CppApi>());
    return instance;
}
