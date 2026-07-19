
#include <thread>

#include "LimeWare.h"

int main() {
    system("am start -n com.axlebolt.standoff2/com.google.firebase.MessagingUnityPlayerActivity > /dev/null 2>&1");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (g().initialize(true)) {
        return 0;
    }
    return 1;
}

void __attribute__((constructor)) entry_point() {
    std::thread([=]() {
        sleep(3);
        g().log->info("Initializing dumper...");
        if (g().initialize(false)) {
            g().log->info("Dumping()...");
            g().dumper->dump("/storage/emulated/0/Android/obb/com.axlebolt.standoff2/");
            g().log->info("Dumped!");
            exit(0);
        }
    }).detach();
}