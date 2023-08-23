# elisa-rpc
### Requirements
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [Gio - 2.0](https://docs.gtk.org/gio/)
- [Discord GameSDK](https://discord.com/developers/docs/game-sdk/sdk-starter-guide#step-1-get-the-thing)

<small>Version 2.5.6 of the Discord GameSDK is bundled in this repository.</small>
### Installation
Edit `config.mk` to match your local setup (`elisa-rpc` is installed into the `/usr/local` namespace by default)

Afterwards, enter the following command to build and install `elisa-rpc` (if necessary as root)
```
# make clean install
```

### Usage
Copy "config-template.json" to `$HOME/.config/elisa-rpc/config.json`
#### Create an application for the RPC
1. Go to https://discord.com/developers/applications
2. Create a new application

#### Put required info in the config file
1. Go to "OAuth2"
2. Copy the Client ID and place it in the "id" field of config.json
3. Add "http://127.0.0.1" to "Redirects"

#### Put optional info in the config file
1. Go to "Rich Presence"
2. Upload your image assets
3. Place the name of your image assets in the "image_key" fields of config.json
4. Add descriptions for your image assets in config.json

<small>Example assets are provided in the `images` directory</small>

### Configuration
| Variable               | Description                                                      |
|------------------------|------------------------------------------------------------------|
| `id`                   | The Client ID of your Discord application. (`int`)               |
| `paused_image_key`     | The image key for the small image while paused. (`str`)          |
| `playing_image_key`    | The image key for the small image while playing. (`str`)         |
| `large_image_key`      | The image key for the large image. (`str`)                       |
| `small_image_text`     | The tooltip for the small image. (`str`)                         |
| `large_image_text`     | The tooltip for the large image. (`str`)                         |
| `swap_small_images`    | Whether or not to switch the playing and paused images. (`bool`) |
| `clear_on_pause`       | Whether or not to clear RPC while paused. (`bool`)               |
| `details`              | The details displayed on the RPC (top). (`str`)                  |
| `state`                | The state displayed on the RPC (bottom). (`str`)                 |

`small_image_text`, `large_image_text`, `details`, and `state`, can all be formatted with the following:
| Key         | Description                                                  |
|-------------|--------------------------------------------------------------|
| `%track%`   | The name of the current track.                               |
| `%title`    | Alias for `%track%`.                                         |
| `%album`    | The name of the album of the current track.                  |
| `%artist%`  | The name of the artist of the current track.                 |
| `%playing%` | The current playback status ("Playing", "Paused", "Stopped") |


This program is licensed under the MIT license.