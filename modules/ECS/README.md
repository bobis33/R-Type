# ECS

The ECS separates **data** from **logic**:

- **Entities** are just **IDs** that represent game objects.  
  They don’t contain logic or data by themselves.

- **Components** are **plain data structures** attached to entities (e.g. position, texture, velocity…).  
  Each component describes a specific aspect of an entity.

- **Systems** contain the **game logic**.  
  They operate on entities that have specific components.

---

## The Registry

The **`ecs::Registry`** is the **core** of the ECS.  
It is responsible for:
- Creating entities
- Adding and removing components
- Accessing component data

Example:
```cpp
ecs::Registry registry;

ecs::Entity player = registry.createEntity()
    .with<ecs::Transform>(0.f, 0.f, 0.f)
    .with<ecs::Velocity>(100.f, 0.f)
    .build();
```

---

## Systems

To add logic to your game, you must implement your own systems by inheriting from eng::ISystem (or eng::ASystem).

Each system defines an update() function that runs every frame.

Example:
```cpp
class MovementSystem : public eng::ASystem {
public:
    void update(ecs::Registry& registry, float dt) override {
        auto& velocities = registry.getAll<ecs::Velocity>();
        for (auto& [entity, vel] : velocities) {
            if (auto* transform = registry.getComponent<ecs::Transform>(entity))
                transform->x += vel.x * dt, transform->y += vel.y * dt;
        }
    }
};
```

---

## Summary
| Concept   | Description                                 |
|:----------|:--------------------------------------------|
| Registry  | Central manager for entities and components |
| Entity    | Unique ID that groups components            |
| Component | Structure holding data only                 |
| System    | Class containing game logic                 |

The ECS is designed to keep the code modular, clear, and easy to extend.
You simply define your data as components and your logic as systems — the registry connects everything.