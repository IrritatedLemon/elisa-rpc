#include "config.hpp"

#include <fstream>
#include <iostream>
#include <unistd.h>

Config get_config() {
    const char* config_home = getenv("XDG_CONFIG_HOME");

    std::string path;

    if (config_home) {
        path = std::string(config_home);
    } else {
        path = std::string(getenv("HOME")) + "/.config";
    }

    path += "/elisa-rpc/config.json";

    std::ifstream f(path, std::ifstream::binary | std::ifstream::ate);

    if (!f.is_open()) {
        std::cout << "[Config] Could not open file '" << path << "': " << std::strerror(errno) << std::endl;
        exit(-1);
    }

    std::string contents;
    contents.resize(f.tellg());

    f.seekg(0, std::ifstream::beg);
    f.read(&contents[0], contents.size());
    f.close();

    nlohmann::json JSON = nlohmann::json::parse(contents);

    Config config;

    try {
        config = {JSON["id"],
                  JSON.value("paused_image_key", ""),
                  JSON.value("playing_image_key", ""),
                  JSON.value("large_image_key", ""),
                  JSON.value("small_image_text", ""),
                  JSON.value("large_image_text", ""),
                  JSON.value("state", ""),
                  JSON.value("details", ""),
                  JSON.value("swap_small_images", false),
                  JSON.value("clear_on_pause", false)};
    } catch (nlohmann::json::type_error& e) {
        if (e.id == 302) {
            std::cout << e.what() << std::endl;
            std::cout << "[Config]: 'id' must have a value." << std::endl;
        }

        exit(-1);
    };

    return config;
}