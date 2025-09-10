# R-Type | Client
you can use a JSON file to configure your build and run options:
```bash
./r-type_client --config r-config-client.json
```

```json
{
  "client": {
    "server_ip": "0.0.0.0",
    "server_port": 4242,
    "player_name": "Player"
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
| Action        | Key          |
|---------------|--------------|
| Move Up       | Z            |
| Move Down     | S            |
| Move Left     | Q            |
| Move Right    | D            |
| Shoot         | Space        |
| Pause         | Escape       |

## Audio
Found original R-Type audio files [here](https://downloads.khinsider.com/game-soundtracks/album/r-type-original-sound-box).

## Inspiration / Examples

- https://www.youtube.com/watch?v=pVWtI0426mU&ab_channel=WorldofLongplays