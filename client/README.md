# R-Type | Client
you can use a JSON file to configure your build and run options:
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

## Key Bindings
| Action     | Key    |
|------------|--------|
| Move Up    | ↑      |
| Move Down  | ↓      |
| Move Left  | ←      |
| Move Right | →      |
| Pause      | Escape |

## Audio
Found original R-Type audio files [here](https://downloads.khinsider.com/game-soundtracks/album/r-type-original-sound-box).

## Inspiration / Examples

- https://www.youtube.com/watch?v=pVWtI0426mU&ab_channel=WorldofLongplays