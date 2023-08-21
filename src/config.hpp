#pragma once

#include <string>

#include <nlohmann/json.hpp>

struct Config {
    int64_t id;
    std::string paused_image_key;
    std::string playing_image_key;
    std::string large_image_key;
    std::string small_image_text;
    std::string large_image_text;

    std::string state;
    std::string details;

    bool swap_small_images;
    bool clear_on_pause;
};

Config get_config();
