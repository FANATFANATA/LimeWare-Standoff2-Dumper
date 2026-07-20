#include "Injection.h"

#include "LimeWare.h"

#include <thread>
#include <KittyMemOp.hpp>
#include <asm-generic/unistd.h>
#include <android/dlext.h>
#include <dlfcn.h>

bool Injection::start()
{
    if (initialize())
    {
        g().log->info("Injection successfully initialized.");
        if (inject())
        {
            g().log->info("LimeWare-Dumper library successfully injected.");
            if (shutdown())
            {
                g().log->info("Injection successfully shutdown.");
                return true;
            }
        }
    }
    return false;
}

bool Injection::initialize()
{
    for (int i = 1; i < 10; i++)
    {
        if (m_pid)
        {
            break;
        }
        m_pid = KittyMemoryEx::getProcessID("com.axlebolt.standoff2");
        if (!m_pid)
        {
            g().log->error("Process ID not found! Attempt number %d...", i + 1);
            sleep(2);
        }
    }

    if (!m_pid)
    {
        g().log->debug("The process ID was not found after 10 attempts! Make sure the process com.axlebolt.standoff2 is running().");
        return false;
    }

    if (!kitty_memory->initialize(m_pid, EK_MEM_OP_IO, true))
    {
        return false;
    }
    m_trace = kitty_memory->trace;

    return m_trace.attach();
}

bool Injection::inject()
{
    uint64_t mmap_address = kitty_memory->elfScanner.findRemoteSymbol("mmap", reinterpret_cast<uintptr_t>(mmap));
    uint64_t munmap_address = kitty_memory->elfScanner.findRemoteSymbol("munmap", reinterpret_cast<uintptr_t>(munmap));
    uint64_t syscall_address = kitty_memory->elfScanner.findRemoteSymbol("syscall", reinterpret_cast<uintptr_t>(syscall));

    if (!syscall_address)
    {
        g().log->error("syscall function not found!");
        return false;
    }
    if (!mmap_address)
    {
        g().log->error("mmap function not found!");
        return false;
    }

    if (!munmap_address)
    {
        g().log->error("munmap function not found!");
        return false;
    }

    auto space_ret = m_trace.callFunction(mmap_address, nullptr, 512, PROT_READ | PROT_WRITE | PROT_EXEC,
                                          MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    uint64_t space = space_ret.result.ptr;

    if (space_ret.status != KT_RP_CALL_SUCCESS || space == (uint64_t)-1)
    {
        g().log->error("Couldn't create injection space!");
        return false;
    }

    std::string space_name = std::string("LimeWare");

    if (!kitty_memory->writeMemStr(space, space_name))
    {
        g().log->error("Couldn't write injection space name!");
        return false;
    }

    uint64_t memfd_id = m_trace.callFunction(syscall_address, __NR_memfd_create, space, 0).result.ptr;
    FILE *memfd = fopen((std::string("/proc/") + std::to_string(m_pid) + std::string("/fd/") + std::to_string(memfd_id)).c_str(), "w");
    if (!memfd)
    {
        g().log->error("Couldn't create memfd!");
        return false;
    }

    char lib_buffer[1024] = {};
    FILE *lib_file = fopen("/data/local/tmp/LimeWare-Standoff2-Dumper.so", "r");
    if (!lib_file)
    {
        g().log->error("\"%s\" not found!", "/data/local/tmp/LimeWare-Standoff2-Dumper.so");
        return false;
    }
    while (const size_t byte = fread(lib_buffer, 1, sizeof(lib_buffer), lib_file))
    {
        fwrite(lib_buffer, 1, byte, memfd);
    }
    fclose(lib_file);
    fclose(memfd);

    android_dlextinfo info = {};
    info.flags |= ANDROID_DLEXT_USE_LIBRARY_FD;
    info.library_fd = memfd_id;

    const uint64_t anonymous = m_trace.callFunction(mmap_address, nullptr, sizeof(info), PROT_READ | PROT_WRITE,
                                                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)
                                   .result.ptr;
    if (!anonymous)
    {
        g().log->error("Couldn't create space in anonymous region!");
        return false;
    }
    kitty_memory->writeMem(anonymous, &info, sizeof(info));

    uint64_t dlopen_address = kitty_memory->elfScanner.findRemoteSymbol("android_dlopen_ext",
                                                                        reinterpret_cast<uintptr_t>(android_dlopen_ext));
    if (!dlopen_address)
    {
        g().log->error("dlopen function not found!");
        return false;
    }

    uint64_t handle = m_trace.callFunction(dlopen_address, space, RTLD_DEFAULT, anonymous).result.ptr;
    if (!handle)
    {
        if (uint64_t dlerror_address = kitty_memory->elfScanner.findRemoteSymbol("dlerror", reinterpret_cast<uintptr_t>(dlerror)))
        {
            uint64_t err = m_trace.callFunction(dlerror_address).result.ptr;
            if (err)
            {
                g().log->debug("dlerror: %s", kitty_memory->readMemStr(err, 128).c_str());
                return false;
            }
            g().log->error("dlerror() result not found!");
            return false;
        }
        g().log->error("dlerror not found!");
        return false;
    }

    return true;
}

bool Injection::shutdown()
{
    return m_trace.detach();
}
