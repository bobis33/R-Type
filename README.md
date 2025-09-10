<p align="center">
  <img src="/assets/icons/icon.png" alt="R-Type Logo" width="326"/>
</p>

# R-Type
The Goal of this project is to implement a multithreaded server and a graphical client for a game called R-Type, using an engine of your own design.

## Supported Platforms
| Platform   | Compiler | Status                                                                                |
|------------|----------|---------------------------------------------------------------------------------------|
| Linux      | g++      | ![Linux](https://img.shields.io/badge/Linux-green?style=flat-square&logo=linux)       |
| macOS      | g++      | ![macOS](https://img.shields.io/badge/macOS-green?style=flat-square&logo=apple)       |
| Windows    | MSVC     | ![Windows](https://img.shields.io/badge/Windows-green?style=flat-square&logo=windows) |

## Project Structure
```
R-Type
│├── assets                  # Game assets (images, sounds, etc.)
│├── client                  # Client source code
│├── documentation           # Project documentation
│├── modules                 # Static libraries for the project
│├── scripts                 # Build and utility scripts
│├── server                  # Server source code
│├── tests                   # Unit and integration tests
└─── third-party             # External libraries as submodules
```

## Prerequisites
Make sure you have the following dependencies installed on your system:

- [CMake 4.0.0](https://cmake.org/)
- [C++23](https://en.cppreference.com/w/cpp/23)

## Clone the project
> [!IMPORTANT]
> When cloning the project, you should also initialize the submodules:
> ```bash
> git clone --recurse-submodules git@github.com:bobis33/R-Type.git
> ```
> If you already cloned the project, you can initialize the submodules with:
> ```bash
> git submodule update --init --recursive
> ```

## Build and Run
### Unix (Linux, macOS)
```bash
./scripts/unix/build.sh release
## Or
cmake -S . -B cmake-build-release -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc
cmake --build cmake-build-release -- -j4
## Then
./cmake-build-release/r-type_client ## client
./cmake-build-release/r-type_server ## server
```

### Windows
```powershell
cmake -S . -B cmake-build-release -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
## Then
cmake-build-release\bin\r-type_client.exe ## client
cmake-build-release\bin\r-type_server.exe ## server
```

## Documentation

API documentation is generated using Doxygen and deployed on [GitHub Pages](https://bobis33.github.io/R-Type/).
You can find the same documentation as PDF [here](documentation/R-Type.pdf).
More specific documentation for each part of the project can be found in their respective directories:
  - [Client documentation](client/README.md)
  - [Server documentation](server/README.md)

## External Libraries
All dependencies are included as submodules in the [third-party](third-party) directory.

## Commit Norms
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