# Engine Module Overview

The **Engine module** is the core of the framework.  
It defines how the game runs, manages scenes, and connects to modular systems such as **audio**, **graphics**, and **networking** through plugin interfaces.

This design allows the engine to stay **independent of specific libraries** — each plugin (SFML, OpenGL, Asio, etc.) can implement its own version of these interfaces.

---

## General Architecture

The engine provides:
- A **central controller** (`Engine`)
- A **scene manager** (`SceneManager`)
- A **scene interface and base implementation** (`IScene`, `AScene`)
- **Plugin interfaces** for major subsystems:
   - `IAudio` for sound management
   - `IRenderer` for graphics and input
   - `INetworkClient` for network communication

---

## Engine

The `Engine` class is the **main entry point** of the framework.  
It holds and controls all the key modules:

- **Audio** (`IAudio`) — manages sound playback and music
- **Renderer** (`IRenderer`) — handles the window, rendering, and user input
- **Network** (`INetworkClient`) — connects the client to the server
- **SceneManager** — stores and switches between game scenes
- **Clock** — keeps track of delta time between frames

Each of these systems is injected into the engine using **factory functions**.  
This allows full flexibility: you can load different plugins at runtime depending on your platform or desired backend.

During each frame, the engine:
1. Updates the clock and retrieves delta time.
2. Clears the window.
3. Updates all systems in the active scene.
4. Calls the scene’s own update logic.
5. Displays the rendered frame.

---

## SceneManager

The `SceneManager` manages all scenes in the engine.  
A **scene** represents a logical state of the game (menu, level, loading screen, etc.).

It provides:
- Storage for all loaded scenes
- Access to the **current active scene**
- Scene switching and registration functions

Scenes are stored as `std::unique_ptr<IScene>`, ensuring automatic cleanup and safe ownership.

---

## IScene and AScene

The `IScene` interface defines the **minimum contract** every scene must respect:
- A name and an ID
- An internal **ECS registry** to manage entities and components
- Methods for `update()`, `event()`, and `updateSystems()`
- Support for adding and managing ECS systems

`AScene` provides a **default implementation** of `IScene` that already includes:
- An internal ECS `Registry`
- A vector of systems
- Automatic ID generation

Custom scenes should inherit from `AScene` and implement their own logic.

---

## Plugin Interfaces

The plugin interfaces define the boundaries between the **engine core** and the **external modules** (graphics, audio, network).  
They all inherit from `utl::IPlugin`, making them loadable at runtime through the plugin manager.

### IAudio
Defines how the engine interacts with the **audio system**.  
It provides methods to:
- Create and play sounds or music
- Adjust volume or looping behavior
- Query playback status

Example: `SFMLAudio` plugin can implement this interface using SFML sound classes.

### IRenderer
Defines the **rendering and input interface**.  
Responsibilities include:
- Creating and managing the game window
- Drawing sprites, text, and basic shapes
- Handling keyboard input and events

Example: an `OpenGLRenderer` or `SFMLRenderer` plugin can implement this.

### INetworkClient
Defines the **client-side network logic** for communication with the game server.  
Responsibilities include:
- Connecting and disconnecting from a server
- Sending and receiving packets and events
- Managing session data and tick synchronization

Example: an `AsioNetworkClient` plugin could implement this using Boost.Asio.

---

## Summary

| Component           | Responsibility                               |
|---------------------|----------------------------------------------|
| **Engine**          | Central controller connecting all subsystems |
| **SceneManager**    | Manages scenes and handles scene switching   |
| **IScene / AScene** | Defines and implements game scenes with ECS  |
| **IAudio**          | Abstract interface for sound management      |
| **IRenderer**       | Abstract interface for rendering and input   |
| **INetworkClient**  | Abstract interface for network communication |
| **utl::IPlugin**    | Common base for dynamically loaded plugins   |

---

## Design Philosophy

This architecture follows a **modular and decoupled** approach:
- The engine never depends on specific libraries
- Plugins implement interfaces and are loaded dynamically
- Each system (audio, render, network) can be replaced independently
- The engine focuses purely on orchestration and game flow

This separation allows you to easily extend or replace any subsystem without modifying the core engine.
