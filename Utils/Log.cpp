//
// Created by alexandr on 25.04.2026.
//

#include "Log.h"

#include <iostream>
#include <android/log.h>

void Log::info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    __android_log_vprint(ANDROID_LOG_INFO, "[LimeWare]", fmt, args);

    va_list args_copy;
    va_copy(args_copy, args);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args_copy);
    std::cout << "[INFO] " << buffer << std::endl;

    va_end(args_copy);
    va_end(args);
}

void Log::debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    __android_log_vprint(ANDROID_LOG_DEBUG, "[LimeWare]", fmt, args);

    va_list args_copy;
    va_copy(args_copy, args);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args_copy);
    std::cout << "[DEBUG] " << buffer << std::endl;

    va_end(args_copy);
    va_end(args);
}

void Log::error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    __android_log_vprint(ANDROID_LOG_ERROR, "[LimeWare]", fmt, args);

    va_list args_copy;
    va_copy(args_copy, args);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args_copy);
    std::cout << "[ERROR] " << buffer << std::endl;

    va_end(args_copy);
    va_end(args);
}

void Log::warning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    __android_log_vprint(ANDROID_LOG_WARN, "[LimeWare]", fmt, args);

    va_list args_copy;
    va_copy(args_copy, args);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args_copy);
    std::cout << "[WARN] " << buffer << std::endl;

    va_end(args_copy);
    va_end(args);
}

void Log::fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    __android_log_vprint(ANDROID_LOG_FATAL, "[LimeWare]", fmt, args);

    va_list args_copy;
    va_copy(args_copy, args);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args_copy);
    std::cout << "[FATAL] " << buffer << std::endl;

    va_end(args_copy);
    va_end(args);
}
