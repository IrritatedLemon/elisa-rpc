#include "metadata.hpp"

#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

static std::map<std::string, std::string> metadata;

static std::string string_array_to_string(GVariant* array) {
    GVariantIter iter;
    gchar* child;

    std::stringstream result;
    std::vector<gchar*> values;

    g_variant_iter_init(&iter, array);

    while (g_variant_iter_next(&iter, "s", &child)) {
        values.push_back(child);
    }

    for (const std::string value : values) {
        if (!result.str().empty()) {
            result << ", ";
        }

        result << value;
    }

    return result.str();
}

static std::string variant_to_string(GVariant* variant) {
    if (g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING) ||
        g_variant_is_of_type(variant, G_VARIANT_TYPE_OBJECT_PATH)) {
        return g_variant_get_string(variant, NULL);
    } else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_INT32)) {
        return std::to_string(g_variant_get_int32(variant));
    } else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_INT64)) {
        return std::to_string(g_variant_get_int64(variant));
    } else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_DOUBLE)) {
        return std::to_string(g_variant_get_double(variant));
    } else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_BOOLEAN)) {
        return (g_variant_get_boolean(variant) == true) ? "true" : "false";
    } else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING_ARRAY)) {
        return string_array_to_string(variant);
    } else {
        return g_variant_get_type_string(variant);
    }
}

static void dict_to_metadata(GVariant* dict) {
    if (dict == NULL) {
        std::cerr << "Dictionary is NULL" << std::endl;
        return;
    }

    GVariantIter iter;
    GVariant* child;
    gchar* key;

    g_variant_iter_init(&iter, dict);

    while (g_variant_iter_next(&iter, "{sv}", &key, &child)) {
        if (g_variant_is_of_type(child, G_VARIANT_TYPE_VARDICT)) {
            dict_to_metadata(child);
        }

        std::string skey = key;
        skey = skey.substr(skey.find_first_of(':') + 1);
        metadata[skey] = variant_to_string(child);

        g_variant_unref(child);
        g_free(key);
    }
}

static void UpdateTrack(Discord* rpc, bool playing, GDBusProxy* properties) {
    int64_t position;
    GVariant* ret;
    GError* error;

    position = 0;
    error = NULL;

    ret = g_dbus_proxy_call_sync(properties, "Get", g_variant_new("(ss)", "org.mpris.MediaPlayer2.Player", "Position"),
                                 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    if (error) {
        g_print("Error getting Position property: %s\n", error->message);
        g_free(error);
    }

    if (ret) {
        GVariant* vposition;
        g_variant_get_child(ret, 0, "v", &vposition);

        if (vposition) {
            position = g_variant_get_int64(vposition);

            g_variant_unref(vposition);
        }

        g_variant_unref(ret);
    }

    rpc->TrackStatusChange(playing, position);
}

static void PropertiesChanged(GDBusProxy* proxy, GVariant* changed_properties, GStrv invalidated_parameters,
                              gpointer user_data) {
    MetadataHandler* handler = (MetadataHandler*)user_data;
    Discord* rpc = handler->GetRPC();

    dict_to_metadata(changed_properties);

    if (g_variant_lookup_value(changed_properties, "PlaybackStatus", G_VARIANT_TYPE_STRING) &&
        metadata.count("PlaybackStatus"))
        UpdateTrack(rpc, metadata["PlaybackStatus"] == "Playing", handler->properties);

    rpc->UpdateActivity(metadata);
}

MetadataHandler::MetadataHandler(Discord* discord) {
    GVariant* ret;

    rpc.reset(discord);

    loop = nullptr;
    gloop = NULL;

    player = NULL;
    properties = NULL;
    error = NULL;
    ret = NULL;

    player =
        g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, "org.mpris.MediaPlayer2.elisa",
                                      "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", NULL, &error);

    if (player == NULL) {
        g_printerr("Error creating player proxy: %s\n", error->message);
        g_error_free(error);
    }

    properties =
        g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, "org.mpris.MediaPlayer2.elisa",
                                      "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", NULL, &error);

    if (properties == NULL) {
        g_printerr("Error creating properties proxy: %s\n", error->message);
        g_error_free(error);

        return;
    }

    ret = g_dbus_proxy_call_sync(properties, "GetAll", g_variant_new("(s)", "org.mpris.MediaPlayer2.Player"),
                                 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    if (error) {
        gint code = error->code;
        gchar* message = g_strdup(error->message);

        g_error_free(error);
        error = NULL;

        if (code == G_DBUS_ERROR_SERVICE_UNKNOWN) {
            std::cerr << "Cannot get track metadata, is Elisa open?" << std::endl;
        } else {
            g_printerr("Error getting properties: %s\n", message);
        }

        g_free(message);
    }

    if (ret) {
        GVariant* dict = g_variant_get_child_value(ret, 0);

        if (dict) {
            dict_to_metadata(dict);
            UpdateTrack(rpc.get(), metadata["PlaybackStatus"] == "Playing", properties);
            rpc->UpdateActivity(metadata);

            g_variant_unref(dict);
        }

        g_variant_unref(ret);
    }

    loop = new std::thread([this]() {
        gloop = g_main_loop_new(NULL, FALSE);

        if (g_signal_connect(player, "g-properties-changed", G_CALLBACK(PropertiesChanged), this) <= 0) {
            std::cout << "Failed to connect to signal PropertiesChanged" << std::endl;
            exit(-1);
        }

        g_main_loop_run(gloop);
    });
}

Discord* MetadataHandler::GetRPC() {
    return rpc.get();
}

void MetadataHandler::Destroy() {
    if (gloop) {
        g_main_loop_quit(gloop);
        gloop = NULL;
    }

    if (loop) {
        loop->join();
        loop = nullptr;
    }

    (void)rpc.release();

    g_object_unref(player);
    g_object_unref(properties);
}