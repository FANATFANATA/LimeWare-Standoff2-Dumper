cmake_minimum_required(VERSION 4.0)
project(LimeWare-Standoff2-Dumper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(FetchContent)

FetchContent_Declare(
        kittymemoryex
        GIT_REPOSITORY https://github.com/MJx0/KittyMemoryEx.git
        GIT_TAG        93a07fe1527717a6da9b0dd40067e02fa626fda2
)

FetchContent_MakeAvailable(KittyMemoryEx)

file(GLOB CORE CONFIGURE_DEPENDS Core/*.cpp)
file(GLOB INJECTION CONFIGURE_DEPENDS Injection/*.cpp)
file(GLOB UTILS CONFIGURE_DEPENDS Utils/*.cpp)
file(GLOB DUMPER CONFIGURE_DEPENDS Dumper/*.cpp)
file(GLOB_RECURSE KITTYMEMORYEX CONFIGURE_DEPENDS ${kittymemoryex_SOURCE_DIR}/KittyMemoryEx/*.cpp)

add_executable(${PROJECT_NAME}
        main.cpp
        ${CORE}
        ${INJECTION}
        ${UTILS}
        ${DUMPER}
        ${KITTYMEMORYEX}
        Utils/il2cpp-classes_newv.h
        Utils/2019.4.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
        Core/
        Injection/
        Utils/
        Dumper/
        ${kittymemoryex_SOURCE_DIR}
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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND adb push "$<TARGET_FILE:${PROJECT_NAME}>" "/storage/emulated/0/${PROJECT_NAME}.so"
        COMMAND adb shell "su -c 'mv /storage/emulated/0/${PROJECT_NAME}.so /data/local/tmp/${PROJECT_NAME}.so'"
        COMMAND adb shell "su -c 'chmod +x /data/local/tmp/${PROJECT_NAME}.so'"
        COMMAND adb shell "su -c '/data/local/tmp/${PROJECT_NAME}.so'"
        COMMENT "Deploying ${PROJECT_NAME} to device..."
        VERBATIM
)
