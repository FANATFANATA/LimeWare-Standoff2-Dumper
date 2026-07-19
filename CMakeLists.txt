cmake_minimum_required(VERSION 4.0)
project(LimeWare-Standoff2-Dumper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(KITTYMEMORYEX_DIR ${CMAKE_SOURCE_DIR}/KittyMemoryEx-main)

set(CORE
    Core/LimeWare.cpp
)
set(INJECTION
    Injection/Injection.cpp
)
set(UTILS
    Utils/Memory.cpp
    Utils/Log.cpp
    Utils/Il2CppApi.cpp
)
set(DUMPER
    Dumper/Dumper.cpp
)
file(GLOB_RECURSE KITTYMEMORYEX CONFIGURE_DEPENDS ${KITTYMEMORYEX_DIR}/KittyMemoryEx/*.cpp)

add_executable(${PROJECT_NAME}
        main.cpp
        ${CORE}
        ${INJECTION}
        ${UTILS}
        ${DUMPER}
        ${KITTYMEMORYEX}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        Core/
        Injection/
        Utils/
        Dumper/
        ${KITTYMEMORYEX_DIR}
)

target_link_options(${PROJECT_NAME} PRIVATE
        -static-libstdc++
        -static-libgcc
        -Wl,--exclude-libs,ALL
        -Wl,--gc-sections
        -s
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        android
        log
        z
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        __ANDROID__
        kUSE_LOGCAT
        kITTYMEMORY_DEBUG

        hexdump_path="/storage/emulated/0/Android/obb/com.axlebolt.standoff2/"
)

option(LIMEWARE_DEPLOY "Deploy to device via adb after build" ON)

if(LIMEWARE_DEPLOY)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND adb push "$<TARGET_FILE:${PROJECT_NAME}>" "/data/local/tmp/${PROJECT_NAME}.so"
    COMMAND adb shell "su -c 'chmod +x /data/local/tmp/${PROJECT_NAME}.so'"
    COMMAND adb shell "su -c '/data/local/tmp/${PROJECT_NAME}.so'"
    COMMENT "Deploying ${PROJECT_NAME} to device..."
    VERBATIM
)
endif()
