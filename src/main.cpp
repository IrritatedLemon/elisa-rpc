#include <chrono>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <map>
#include <thread>

#include "discord.hpp"
#include "metadata.hpp"

volatile bool interrupted = false;

int main(int argc, char* argv[]) {
    Discord rpc;
    MetadataHandler metadata_handler(&rpc);

    std::signal(SIGINT, [](int) { interrupted = true; });

    while (!interrupted) {
        rpc.RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    metadata_handler.Destroy();
    rpc.Destroy();

    return 0;
}
