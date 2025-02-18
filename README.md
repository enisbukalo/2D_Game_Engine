# GameEngine

A modern C++ 2D game engine built with SFML, featuring an Entity Component System (ECS) architecture.

## Table of Contents
- [GameEngine](#gameengine)
  - [Table of Contents](#table-of-contents)
  - [Notes](#notes)
  - [Features](#features)
    - [Entity Component System (ECS)](#entity-component-system-ecs)
    - [Physics System](#physics-system)
    - [Serialization System](#serialization-system)
    - [Code Organization](#code-organization)
    - [Core Systems](#core-systems)
    - [Math Utilities](#math-utilities)
  - [Dependencies](#dependencies)
  - [Building the Project](#building-the-project)
    - [Build Options](#build-options)
    - [Examples](#examples)
    - [Build Output](#build-output)
    - [Building Example Project](#building-example-project)
      - [NOTE!!!: YOU MUST RUN THE BUILD.SH SCRIPT IN THE ROOT DIRECTORY FIRST.](#note-you-must-run-the-buildsh-script-in-the-root-directory-first)
    - [Dependencies](#dependencies-1)
  - [Usage Example](#usage-example)
  - [Project Structure](#project-structure)
  - [Contributing](#contributing)
  - [License](#license)

## Notes


## Features

### Entity Component System (ECS)
- **Entity Management**: Flexible entity creation and lifecycle management
- **Component-Based Architecture**: Modular component system for easy extension
- **Built-in Components**:
  - `CTransform`: Handles position, velocity, scale, and rotation data storage
  - `CGravity`: Implements basic gravity physics configuration
  - `CName`: Provides naming functionality for entities

### Physics System
- **S2DPhysics**: Centralized physics system that handles all physics calculations
  - Authoritative source for position and velocity updates
  - Implements standard physics equations:
    - Velocity: v = v0 + at
    - Position: p = p0 + v0t + (1/2)at²
  - Handles gravity effects on entities
  - Clear separation between data (`CTransform`, `CGravity`) and behavior (`S2DPhysics`)
  - Efficient batch processing of physics entities

### Serialization System
- **JSON-based Serialization**: Full support for saving and loading game states
- **Component Serialization**: Each component implements serialize/deserialize methods
- **File Utilities**: Robust file I/O operations with error handling
- **Supported Operations**:
  - Save/Load individual entities
  - Save/Load entire game scenes
  - Component state persistence
  - Error handling for file operations

### Code Organization
The codebase is organized using pragma regions for better readability:
```cpp
#pragma region Variables
// Variables are grouped here
#pragma endregion

#pragma region Methods
// Methods are grouped here
#pragma endregion

#pragma region Templates
// Template implementations are grouped here
#pragma endregion
```

### Core Systems
- **Entity Manager**: Handles entity lifecycle, querying, and component management
- **Scene Manager**: Manages game scenes and scene transitions
  - Load/save scenes from/to files
  - Handle scene transitions
  - Error handling for scene operations
  - Scene state management
- **2D Physics System**: Manages physics simulation and calculations
  - Centralized physics processing
  - Gravity and force application
  - Velocity and position updates
  - Component-based physics integration
- **Component Factory**: Provides a factory pattern for component creation
- **JSON System**:
  - `JsonBuilder`: Constructs JSON data structures
  - `JsonParser`: Parses JSON strings
  - `JsonValue`: Represents JSON data types
- **File System**: Handles file I/O with error checking

### Math Utilities
- **Vec2**: A 2D vector class with common operations:
  - Vector arithmetic (add, subtract, multiply, divide)
  - Normalization
  - Rotation
  - Distance calculations

## Dependencies
- SFML 2.6.x: Graphics and window management
- Dear ImGui 1.91.x: Immediate mode GUI
- ImGui-SFML 2.6.x: SFML backend for Dear ImGui
- C++17 or late
- cppcheck
- clang

## Building the Project

The project includes a build script (`build.sh`) that handles the build process, including dependency management, testing, and packaging.

### Build Options

The build script (`build.sh`) provides several options:

- `-h, --help`: Show help message
- `-t, --type TYPE`: Set build type (Debug/Release)
- `-s, --shared`: Build as shared library [default: ON]
- `--no-tests`: Skip building and running tests
- `-c, --clean`: Clean build directory
- `-p, --package`: Create distributable package

### Examples

Clean build with shared library:
```bash
./build.sh -c -s
```

Build package:
```bash
./build.sh -c -s -p
```

Build without running tests:
```bash
./build.sh -c -s --no-tests
```

### Build Output

The build script will:
1. Check for CMakeLists.txt and validate build directory
2. Configure the project with CMake
3. Build all dependencies (SFML, ImGui, etc.)
4. Build the main GameEngine library
5. Run tests (if enabled)
6. Create a package (if requested) with:
   - Headers in `package/include/`
   - Library files in `package/lib/`
   - Binary files in `package/bin/`
   - Copy package to example project

### Building Example Project

There is a build script in the example project that will build the GameEngine package and copy it to the example project.
``` bash
cd example_project
./build_example.sh
```
#### <u><b>NOTE!!!: YOU MUST RUN THE BUILD.SH SCRIPT IN THE ROOT DIRECTORY FIRST.</b></u>

### Dependencies
The build script automatically handles the following dependencies:
- SFML (Graphics, Window, System)
- Dear ImGui
- ImGui-SFML

Dependencies are dynamically linked by default. The shared libraries will be included in the package's bin directory.
You will be required to link the dependencies manually in your project.

## Usage Example
```cpp
// Get the scene manager instance
auto& sceneManager = SceneManager::instance();

// Create a new scene with a physics-enabled entity
auto& entityManager = EntityManager::instance();
auto& physics = S2DPhysics::instance();

auto player = entityManager.addEntity("player");
auto transform = player->addComponent<CTransform>();
auto gravity = player->addComponent<CGravity>();

// Configure initial conditions
transform->setPosition(Vec2(100.0f, 200.0f));
transform->setVelocity(Vec2(5.0f, 0.0f));  // Initial horizontal velocity
gravity->setForce(Vec2(0.0f, -9.81f));     // Standard gravity

// Game loop
float deltaTime = 1.0f / 60.0f;  // 60 FPS
physics.update(deltaTime);        // Physics system updates positions and velocities
entityManager.update(deltaTime);  // Entity system processes updates

// Save the scene
sceneManager.saveScene("level1.json");

// Later, load the scene
sceneManager.loadScene("level1.json");

// Save changes to current scene
sceneManager.saveCurrentScene();

// Clear the scene when done
sceneManager.clearScene();
```

## Project Structure
```
2D_Game_Engine/
├── include/          # Public headers
│   ├── Entity.h
│   ├── EntityManager.h
│   ├── ComponentFactory.h
│   └── utility/
│       ├── FileUtilities.h
│       ├── JsonBuilder.h
│       ├── JsonParser.h
│       └── JsonValue.h
├── components/       # Component implementations
│   ├── CTransform.h
│   ├── CGravity.h
│   └── CName.h
├── systems/         # System implementations
│   ├── System.h
│   └── S2DPhysics.h
└── src/            # Source files
```

## Contributing
Contributions are welcome! Please feel free to submit pull requests.

## License
MIT License