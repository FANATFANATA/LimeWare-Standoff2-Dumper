//
// Created by alexandr on 25.04.2026.
//

#ifndef LIMEWARE_STANDOFF2_DUMPER_INJECTION_H
#define LIMEWARE_STANDOFF2_DUMPER_INJECTION_H

#include <sys/types.h>

#include "KittyMemoryEx/KittyMemoryMgr.hpp"

class Injection {

    pid_t m_pid = 0;
    KittyTraceMgr m_trace;
    std::unique_ptr<KittyMemoryMgr> kitty_memory;

public:
    explicit Injection(std::unique_ptr<KittyMemoryMgr> memory) : kitty_memory(std::move(memory)) {}
    ~Injection() = default;

    bool start();
    bool initialize();
    bool inject();
    bool shutdown();
};


#endif //LIMEWARE_STANDOFF2_DUMPER_INJECTION_H