# R-Type | Server

you can use a JSON file to configure your build and run options:
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