# GameEngine

A modern C++ 2D game engine built with SFML, featuring an Entity Component System (ECS) architecture.

## Table of Contents
- [GameEngine](#gameengine)
  - [Table of Contents](#table-of-contents)
  - [Notes](#notes)
  - [Features](#features)
    - [Entity Component System (ECS)](#entity-component-system-ecs)
    - [Physics Scale Convention](#physics-scale-convention)
    - [Physics System](#physics-system)
    - [Serialization System](#serialization-system)
    - [Code Organization](#code-organization)
    - [Core Systems](#core-systems)
    - [Math Utilities](#math-utilities)
  - [Dependencies](#dependencies)
  - [Building the Project](#building-the-project)
    - [Docker Build (Recommended)](#docker-build-recommended)
- [Build and run tests](#build-and-run-tests)
- [Build without tests](#build-without-tests)
- [Clean build](#clean-build)
- [Build for Windows](#build-for-windows)
    - [Build Options](#build-options)
    - [Examples](#examples)
- [Clean build with tests:](#clean-build-with-tests)
- [Release build without tests:](#release-build-without-tests)
- [Build as shared library:](#build-as-shared-library)
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
  - `CPhysicsBody2D`: Box2D physics body wrapper (Dynamic, Kinematic, or Static)
  - `CCollider2D`: Box2D collision shape wrapper (Circle or Box)
  - `CName`: Provides naming functionality for entities

### Physics Scale Convention
- **100 pixels = 1 meter**: Conversion scale for rendering Box2D physics
- **Box2D Coordinates**: Y-up (positive Y = upward), units in meters
- **Screen Coordinates**: Y-down (positive Y = downward), units in pixels
- **Default gravity**: 9.81 m/s² downward in Box2D (equivalent to Earth's gravity)

### Physics System
- **SBox2DPhysics**: Box2D v3.0 integration with ECS architecture
  - Industry-standard 2D physics engine with native API
  - Manages Box2D world and physics simulation
  - Automatic synchronization between Box2D and CTransform components
  - **Physics Bodies** (`CPhysicsBody2D`):
    - Dynamic: Affected by forces and gravity
    - Kinematic: Controlled by velocity, not forces
    - Static: Immovable objects (ground, walls)
    - Properties: density, friction, restitution, damping, gravity scale
  - **Collision Shapes** (`CCollider2D`):
    - Circle shapes with configurable radius
    - Box shapes with configurable width and height
    - Material properties: density, friction, restitution
  - **Forces and Impulses**:
    - Apply forces: continuous acceleration
    - Apply impulses: instant velocity changes
    - Angular and linear control
  - Clear separation between physics simulation (Box2D) and rendering (SFML)

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
  - Automatic component initialization after deserialization
- **Box2D Physics System**: Manages physics simulation and calculations
  - Centralized Box2D world management
  - Gravity and force application
  - Collision detection and response
  - Component-based physics integration
  - Automatic sync between Box2D and ECS components
- **Component Factory**: Provides a factory pattern for component creation
  - Registers all built-in components
  - Supports custom component registration
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
- SFML 2.6.1: Graphics and window management
- Box2D v3.0: 2D physics engine
- Dear ImGui 1.88: Immediate mode GUI
- ImGui-SFML 2.6: SFML backend for Dear ImGui
- C++17 or later
- CMake 3.28+
- Google Test (for testing)

## Building the Project

The project supports building both for **Linux** (native) and **Windows** (cross-compilation) via Docker.

### Docker Build (Recommended)

**Prerequisites:**
- Docker
- Docker Compose
- Run Docker Container
- - ```docker-compose up -d```

**Linux Build (Development/Testing):**

# Build and run tests
```docker-compose exec dev ./build_linux.sh```

# Build without tests
```docker-compose exec dev ./build_linux.sh --no-tests```

# Clean build
```docker-compose exec dev ./build_linux.sh --clean```


**Windows Build (Cross-compilation):**

# Build for Windows
```docker-compose exec dev ./build_windows.sh```


**Enter Development Environment:**

```docker-compose exec dev /bin/bash```


### Build Options

The build script (`build.sh`) provides several options:

- `-h, --help`: Show help message
- `-t, --type TYPE`: Set build type (Debug/Release) [default: Debug]
- `-s, --shared`: Build as shared library [default: OFF]
- `--no-tests`: Skip building and running tests
- `-c, --clean`: Clean build directory
- `-i, --install-prefix`: Set install prefix [default: ./package]

### Examples

# Clean build with tests:

```./build.sh --clean```


# Release build without tests:

```./build.sh -t Release --no-tests```


# Build as shared library:

```./build.sh -s```


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

// Set global gravity (affects all entities with CGravity)
// Scale: 100 pixels = 1 meter, so 9.81 m/s² = 981 px/s²
// Positive Y = downward (screen coordinates)
physics.setGlobalGravity(Vec2(0.0f, 981.0f));  // Standard Earth gravity

auto player = entityManager.addEntity("player");
auto transform = player->addComponent<CTransform>();
auto gravity = player->addComponent<CGravity>();  // Uses default 1.0 multiplier

// Configure initial conditions
transform->setPosition(Vec2(100.0f, 200.0f));
transform->setVelocity(Vec2(5.0f, 0.0f));  // Initial horizontal velocity

// Optional: Customize gravity for specific entities
auto moon = entityManager.addEntity("moon");
auto moonTransform = moon->addComponent<CTransform>();
auto moonGravity = moon->addComponent<CGravity>();
moonGravity->setMultiplier(0.166f);  // Moon has ~1/6 Earth's gravity

// Game loop
float deltaTime = 1.0f / 60.0f;  // 60 FPS
physics.update(deltaTime);        // Physics system updates positions and velocities
entityManager.update(deltaTime);  // Entity system processes updates

// Change gravity globally (e.g., zero gravity sequence)
physics.setGlobalGravity(Vec2(0.0f, 0.0f));

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