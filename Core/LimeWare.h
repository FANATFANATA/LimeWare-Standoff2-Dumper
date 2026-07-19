#ifndef LIMEWARE_STANDOFF2_DUMPER_LIMEWARE_H
#define LIMEWARE_STANDOFF2_DUMPER_LIMEWARE_H

#include <memory>

#include "Log.h"
#include "Injection.h"
#include "Dumper.h"
#include "Il2CppApi.h"
#include "Memory.h"

class LimeWare {
public:
    std::unique_ptr<Log> log;
    std::unique_ptr<Injection> injection;
    std::unique_ptr<Dumper> dumper;
    std::unique_ptr<Memory> memory;
    std::unique_ptr<Il2CppApi> il2cpp;

    explicit LimeWare(std::unique_ptr<Log> log, std::unique_ptr<Injection> injection, std::unique_ptr<Dumper> dumper,
        std::unique_ptr<Memory> memory, std::unique_ptr<Il2CppApi> il2cpp) :
        log(std::move(log)), injection(std::move(injection)), dumper(std::move(dumper)),
        memory(std::move(memory)), il2cpp(std::move(il2cpp)) {}
    ~LimeWare() = default;

    bool initialize(bool inject);
};

LimeWare& g();

#endif