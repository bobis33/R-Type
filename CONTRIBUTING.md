# Contributing to R-Type

## Code Style

### C++
 - Use **clang-format** with the provided configuration
 - Follow RAII principles
 - Use `std::unique_ptr`/`std::shared_ptr` instead of raw pointers
 - Header files: `.hpp` - Implementation : `.cpp`
 - Namespace prefixes:
   - `eng::` → Engine
   - `ecs::` → ECS
   - `cli::` → Client
   - `gme::` → Game logic
   - `utl::` → Utils library

#### Naming Convention 
- Classes / Structs / Enums: `PascalCase`
```cpp
class PlayerEntity { ... };
struct TransformComponent { ... };
```
- Methods / Variables / Functions: `camelCase`
```cpp
void updatePosition();
int playerHealth;
```
- Constants / Macros: `UPPER_CASE_SNAKE_CASE`
```cpp
const int MAX_PLAYERS = 4;
```
- Namespaces: `lowercase`
```cpp
namespace ecs { ... }
```

### Yaml
- Extention : `.yml`

## Contribution workflow
We strictly use **GitHub Issues** and **Pull Requests** (PRs).
No direct commits to `main` are allowed.

### Create / Find an Issue
Before contributing, make sure there is an **open issue** for your task.
If not, **create one** using the provided templates:

- **Bug Report**
- **Feature Request**

### Branching & PR Naming
Each contribution must be developed in a dedicated branch.
Branches and Pull Requests must follow this naming convention:
```
<Type>/<Scope>/<Sub-scope>
```
- **Type** → `Feature` | `Fix` | `Refactor` | `Docs` | `Test` | `CI`
- **Scope** → `Client` | `Server`
- **Sub-scope** → `Engine` | `ECS` | `Game` | etc.

This ensures consistency across branches and PRs, making it easy to identify the purpose of each contribution at a glance.

### Commit Norms
| Commit Type | Description                                                                                                               |
|:------------|:--------------------------------------------------------------------------------------------------------------------------|
| build       | Changes that affect the build system or external dependencies (npm, make, etc.)                                           |
| ci          | Changes related to integration files and scripts or configuration (Travis, Ansible, BrowserStack, etc.)                   |
| feat        | Addition of a new feature                                                                                                 |
| fix         | Bug fix                                                                                                                   |
| perf        | Performance improvements                                                                                                  |
| refactor    | Modification that neither adds a new feature nor improves performance                                                     |
| style       | Change that does not affect functionality or semantics (indentation, formatting, adding space, renaming a variable, etc.) |
| docs        | Writing or updating documentation                                                                                         |
| test        | Addition or modification of tests                                                                                         |