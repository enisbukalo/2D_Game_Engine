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
  - [Audio Attribution](#audio-attribution)
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
  - `CCollider2D`: Box2D collision shape wrapper (Circle, Box, Polygon, or Segment)
  - `CName`: Provides naming functionality for entities
  - `CInputController`: Entity-specific input handling with action bindings
  - `CAudioListener`: Marks entity as audio listener for spatial audio
  - `CAudioSource`: Enables audio playback on entities (2D or spatial)

### Physics Scale Convention
- **100 pixels = 1 meter**: Conversion scale for rendering Box2D physics
- **Box2D Coordinates**: Y-up (positive Y = upward), units in meters
- **Screen Coordinates**: Y-down (positive Y = downward), units in pixels
- **Default gravity**: 9.81 m/s² downward in Box2D (equivalent to Earth's gravity)

### Physics System
- **SBox2DPhysics**: Box2D v3.1.1 integration with ECS architecture
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
- **Audio System (SAudioSystem)**: SFML-based audio engine with advanced features
  - **Music Streaming**: Single streamed music track with seamless playback
  - **SFX Pooling**: Efficient sound effect management with configurable pool size (default 32)
  - **Spatial Audio**: 3D positioned sound effects with distance attenuation
  - **Fade Effects**: Smooth fade-in/fade-out transitions for music and SFX
    - Linear and exponential fade curves
    - Configurable fade duration
  - **Volume Control**: Independent master, music, and SFX volume levels
  - **Looping**: Support for both looping and one-shot playback
  - **Pitch Control**: Adjust playback speed and pitch
  - **Audio Handle System**: Track and control playing sounds
  - Access via `gameEngine.getAudioSystem()`
- **Input Manager (SInputManager)**: Comprehensive input handling system
  - Keyboard and mouse input abstraction
  - Action binding system for gameplay events
  - Entity-specific input controllers via `CInputController` component
  - Support for pressed, released, and held states
  - Access via `gameEngine.getInputManager()`
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
- Box2D v3.1.1: 2D physics engine
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

## API Usage

### Accessing Engine Systems

The GameEngine class provides the recommended public API for accessing all engine systems and managers. Instead of directly calling singleton `instance()` methods, use the GameEngine accessor methods:

```cpp
GameEngine gameEngine(&window, sf::Vector2f(0.0f, -9.81f));

// Access systems through GameEngine (recommended)
auto& entityManager = gameEngine.getEntityManager();
auto& physics = gameEngine.getPhysics();
auto& audioSystem = gameEngine.getAudioSystem();
auto& inputManager = gameEngine.getInputManager();
auto& sceneManager = gameEngine.getSceneManager();
auto& componentFactory = gameEngine.getComponentFactory();
```

### Simplified Include Paths

The engine uses a flat include structure. You can include headers directly without subdirectory prefixes:

```cpp
// Components - no "components/" prefix needed
#include <CTransform.h>
#include <CPhysicsBody2D.h>
#include <CCollider2D.h>
#include <CInputController.h>
#include <CAudioSource.h>

// Systems - no "systems/" prefix needed
#include <SBox2DPhysics.h>
#include <SAudioSystem.h>
#include <SInputManager.h>

// Entities and Managers
#include <Entity.h>
#include <EntityManager.h>
#include <SceneManager.h>

// Utilities
#include <Vec2.h>
#include <JsonBuilder.h>
```

## Usage Example
```cpp
#include <GameEngine.h>
#include <Entity.h>
#include <CTransform.h>
#include <CPhysicsBody2D.h>
#include <CCollider2D.h>
#include <Vec2.h>
#include <SFML/Graphics.hpp>

int main()
{
    // Create window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Game");
    
    // Create game engine with gravity
    GameEngine gameEngine(&window, sf::Vector2f(0.0f, -9.81f));
    
    // Access systems through GameEngine (recommended API)
    auto& entityManager = gameEngine.getEntityManager();
    auto& physics = gameEngine.getPhysics();
    auto& sceneManager = gameEngine.getSceneManager();
    auto& audioSystem = gameEngine.getAudioSystem();

    // Initialize Box2D world with gravity
    // Note: Box2D uses Y-up coordinates (positive Y = upward) and meters
    physics.setGravity({0.0f, -10.0f});  // Standard Earth gravity (9.81 m/s²)

    // Create a dynamic physics entity (e.g., player)
    auto player = entityManager.addEntity("player");
    auto transform = player->addComponent<CTransform>();
    auto physicsBody = player->addComponent<CPhysicsBody2D>();
    auto collider = player->addComponent<CCollider2D>();

    // Initialize physics body at starting position (in meters)
    transform->setPosition(Vec2(5.0f, 10.0f));  // Position in meters
    physicsBody->initialize({5.0f, 10.0f}, BodyType::Dynamic);

    // Add a circle collider (radius in meters)
    collider->createCircle(0.5f);  // 0.5 meter radius
    collider->setDensity(1.0f);    // 1 kg/m²
    collider->setFriction(0.3f);
    collider->setRestitution(0.5f); // 50% bounciness

    // Create a static ground platform
    auto ground = entityManager.addEntity("ground");
    auto groundTransform = ground->addComponent<CTransform>();
    auto groundBody = ground->addComponent<CPhysicsBody2D>();
    auto groundCollider = ground->addComponent<CCollider2D>();

    groundTransform->setPosition(Vec2(10.0f, 1.0f));
    groundBody->initialize({10.0f, 1.0f}, BodyType::Static);
    groundCollider->createBox(10.0f, 0.5f);  // 10m wide, 0.5m tall

    // Apply forces and impulses
    physicsBody->applyLinearImpulseToCenter({5.0f, 0.0f});  // Horizontal kick
    physicsBody->applyForceToCenter({0.0f, 100.0f});        // Upward force

    // Customize physics properties
    physicsBody->setGravityScale(0.5f);      // Half gravity
    physicsBody->setLinearDamping(0.1f);     // Air resistance
    physicsBody->setAngularDamping(0.1f);    // Rotation damping

    // Initialize audio system and load sounds
    audioSystem.initialize();
    audioSystem.loadSound("background_music", "assets/music.mp3", AudioType::Music);
    audioSystem.loadSound("jump_sfx", "assets/jump.wav", AudioType::SFX);
    
    // Play background music with looping
    audioSystem.playMusic("background_music", true, 0.7f);
    
    // Play sound effect with fade-in
    FadeConfig fadeIn = FadeConfig::linear(1.0f, true);
    AudioHandle jumpHandle = audioSystem.playSFXWithFade("jump_sfx", 1.0f, 1.0f, false, fadeIn);

    // Game loop
    float deltaTime = 1.0f / 60.0f;
    while (window.isOpen())
    {
        gameEngine.update(deltaTime);
        gameEngine.render();
    }

    // Save the scene (includes all physics state)
    sceneManager.saveScene("level1.json");

    // Later, load the scene (automatically recreates Box2D bodies)
    sceneManager.loadScene("level1.json");

    // Clear the scene when done
    sceneManager.clearScene();
    
    return 0;
}
```

## Project Structure
```
2D_Game_Engine/
├── include/          # Public headers
│   ├── Entity.h
│   ├── EntityManager.h
│   ├── ComponentFactory.h
│   ├── SceneManager.h
│   ├── components/
│   │   ├── CTransform.h
│   │   ├── CPhysicsBody2D.h
│   │   ├── CCollider2D.h
│   │   └── CName.h
│   ├── systems/
│   │   ├── System.h
│   │   ├── SBox2DPhysics.h
│   │   └── Box2DContactListener.h
│   └── utility/
│       ├── FileUtilities.h
│       ├── JsonBuilder.h
│       ├── JsonParser.h
│       └── JsonValue.h
└── src/            # Source files
```

## Audio Attribution

Music tracks used in this project are provided under royalty-free licenses and require attribution:

- **Sway** by Yunior Arronte  
  License: Royalty Free Music (Free with Attribution)  
  Source: [Download Link - requires attribution text from site]  
  Description: Gentle Lofi featuring piano, percussion and drums

- **Rainy Day** by Yunior Arronte  
  License: Royalty Free Music (Free with Attribution)  
  Source: [Download Link - requires attribution text from site]  
  Streaming: [Spotify](https://open.spotify.com/track/3qN47D55JWf14GQIMEDT1d) | [Apple Music](https://music.apple.com/us/album/rainy-day-single/1735587688) | [YouTube Music](https://music.youtube.com/watch?v=ZFSkcUDWlhl) | [Amazon Music](https://music.amazon.in/albums/B0CXV388LS) | [Deezer](https://deezer.page.link/K2QkQBGPpoPWnCve9)  
  Description: Lo-fi Jazz featuring jazzy piano, calming drums and bass

- **Thai motor boat** by jonny4c (Freesound)  
  License: Pixabay Content License (Free, No Attribution Required)  
  Source: https://pixabay.com/sound-effects/  
  Description: Transportation, Island, Motorboat sound effect

**Pixabay License Notes (Thai motor boat):**
- ✅ Free to use without attribution (attribution appreciated but not required)
- ✅ Can be modified or adapted
- ❌ Cannot sell or distribute as standalone content
- ❌ Cannot use in immoral or illegal ways
- ❌ Cannot use as part of trademarks or business names

**Royalty Free Music License Restrictions (Sway & Rainy Day):**
- ❌ No use in podcasts
- ❌ No audiobook creation
- ❌ No music remixing or derivative musical works
- ✅ Free to use in games and videos with attribution

## Contributing
Contributions are welcome! Please feel free to submit pull requests.

## License
MIT License