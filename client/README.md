# R-Type | Client

This document details how to configure and run the R-Type client.

---

## Configuration

You can configure the client using a JSON file:
```bash
./r-type_client --config config-client.json
```
Here is an example configuration file:
```json
{
  "client": {
    "server_ip": "0.0.0.0",
    "server_port": 4242,
    "player_name": "Player"
  },
  "plugins": {
    "audio": "my_audio_plugin.so",
    "network": "my_network_client_plugin.so",
    "renderer": "my_renderer_plugin.so",
    "game_solo": "my_game_client_solo_plugin.so",
    "game_multi": "my_game_client_multi_plugin.so"
  },
  "window": {
    "fullscreen": false,
    "width": 920,
    "height": 540,
    "frame_limit": 240
  }
}
```

## Plugins

The client uses five main plugin types:

- **Audio**: Implements `IAudio`
- **GameSolo**: Implements `IGameClient`
- **GameMulti**: Implements `IGameClient`
- **Renderer**: Implements `IRenderer`
- **Network client**: Implements `INetworkClient` 

Refer to the plugin folder and the interfaces documentation to implement custom plugins.

## Troubleshooting / Tips

 - Ensure your JSON paths are correct for plugins.