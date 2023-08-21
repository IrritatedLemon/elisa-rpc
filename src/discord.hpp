#pragma once

#include <map>
#include <memory>

#include <discord/discord.h>

#include "config.hpp"

class Discord {
  public:
    Discord();

    void UpdateActivity(std::map<std::string, std::string> metadata);
    void RunCallbacks();
    void TrackStatusChange(bool playing, int64_t position);
    void Destroy();

    discord::Core* core;

  private:
    discord::Activity activity;
    Config config;

    bool is_init;
    bool playing;

    void Init();
};