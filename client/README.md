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
    "audio": "/plugins/my_audio_plugin.so",
    "network": "/plugins/my_network_client_plugin.so",
    "renderer": "/plugins/my_renderer_plugin.so"
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

The client uses three main plugin types:

- **Audio**: Implements `IAudio`
- **Renderer**: Implements `IRenderer`
- **Network client**: Implements `INetworkClient` 

Refer to the plugin folder and the interfaces documentation to implement custom plugins.

## Troubleshooting / Tips

 - Ensure your JSON paths are correct for plugins.