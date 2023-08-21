#include "discord.hpp"

#include <chrono>
#include <iostream>

void format_string(std::string& subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;

    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

Discord::Discord() {
    config = get_config();

    core = nullptr;

    Init();

    memset(&activity, 0, sizeof(activity));

    activity.SetType(discord::ActivityType::Playing);
    activity.GetAssets().SetLargeImage(config.large_image_key.c_str());
    activity.GetAssets().SetLargeText(config.large_image_text.c_str());
    activity.GetAssets().SetSmallImage(config.small_image_text.c_str());
    activity.GetAssets().SetSmallText(config.small_image_text.c_str());
}

void Discord::UpdateActivity(std::map<std::string, std::string> metadata) {
    std::map<std::string, std::string> status;
    std::map<std::string, std::string> format;

    status["details"] = config.details;
    status["state"] = config.state;
    status["small_image"] = config.small_image_text;
    status["large_image"] = config.large_image_text;

    /* %(key)% = metadata[value] */
    format["track"] = "title";
    format["title"] = "title";
    format["album"] = "album";
    format["artist"] = "artist";
    format["playing"] = "PlaybackStatus";

    for (auto& key : status) {
        for (auto& fmt : format) {
            format_string(key.second, '%' + fmt.first + '%', metadata[fmt.second]);
        }
    }

    activity.SetDetails(status["details"].c_str());
    activity.SetState(status["state"].c_str());
    activity.GetAssets().SetSmallText(status["small_image"].c_str());
    activity.GetAssets().SetLargeText(status["large_image"].c_str());

    if ((!config.clear_on_pause || playing) && is_init) {
        core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
            if (result != discord::Result::Ok) {
                std::cerr << "Failed updating activity, code: " << static_cast<int>(result) << std::endl;
            }
        });
    }
}

void Discord::TrackStatusChange(bool playing, int64_t position) {
    const bool use_paused_image = (!playing || config.swap_small_images);
    int64_t now = std::chrono::seconds(std::time(NULL)).count();
    this->playing = playing;

    if (playing) {
        activity.GetTimestamps().SetStart(now - (position / 10e5));
    } else {
        memset(&activity.GetTimestamps(), 0, sizeof(discord::ActivityTimestamps));

        if (config.clear_on_pause) {
            core->ActivityManager().ClearActivity([](discord::Result result) {
                if (result != discord::Result::Ok) {
                    std::cerr << "Failed clearing activity, code: " << static_cast<int>(result) << std::endl;
                }
            });
        }
    }

    activity.GetAssets().SetSmallImage(use_paused_image ? config.paused_image_key.c_str()
                                                        : config.playing_image_key.c_str());
}

void Discord::RunCallbacks() {
    if (is_init) {
        core->RunCallbacks();
    } else {
        std::cerr << "Discord has not been instantiated." << std::endl;
    }
}

void Discord::Destroy() {
    RunCallbacks();

    if (is_init) {
        delete core;
        core = nullptr;

        is_init = false;
    } else {
        std::cerr << "Discord is already uninstantiated!" << std::endl;
    }
}

void Discord::Init() {
    if (core) {
        delete core;
        core = nullptr;
    }

    auto result = discord::Core::Create(config.id, DiscordCreateFlags_NoRequireDiscord, &core);

    if (core == nullptr) {
        std::cerr << "Failed to instantiate Discord core! (Code: " << static_cast<int>(result) << ")" << std::endl;
        std::exit(-1);
    }

    core->SetLogHook(discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
        std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << std::endl;
    });

    is_init = true;
}