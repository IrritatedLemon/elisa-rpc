#pragma once

#include <map>
#include <string>
#include <thread>

#include <gio/gio.h>

#include "discord.hpp"

class MetadataHandler {
  public:
    MetadataHandler(Discord* rpc);

    Discord* GetRPC();
    void Destroy();

    GDBusProxy* properties;

  private:
    GDBusProxy* player;

    GMainLoop* gloop;
    GError* error;

    std::unique_ptr<Discord> rpc;
    std::thread* loop;
};