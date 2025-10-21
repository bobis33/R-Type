# R-Type | Server

This document details how to configure and run the R-Type server.

---

## Configuration

You can configure the server using a JSON file:
```bash
./r-type_server --config config-server.json
```
Here is an example configuration file:
```json
{
  "host": "0.0.0.0",
  "port": 2560,
  "plugins": {
    "network": "/plugins/my_network_server_plugin.so"
  }
}
```

## Plugins

The server requires a network plugin implementing `INetworkServer`.

Refer to the plugin folder and the interfaces documentation to implement custom plugins.

## Networking / Protocol

- The server communicates with clients using **UDP**.
- Clients must use the correct server IP and port.
- Session management, player events, and game state updates are handled through the network plugin.

## Troubleshooting / Tips

- Ensure your JSON paths are correct for plugins.