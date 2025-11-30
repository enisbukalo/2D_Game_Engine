# GameEngine

A modern C++ 2D game engine built with SFML, featuring an Entity Component System (ECS) architecture.

## Table of Contents
- [GameEngine](#gameengine)
  - [Table of Contents](#table-of-contents)
  - [Notes](#notes)
  - [Features](#features)
    - [Entity Component System (ECS)](#entity-component-system-ecs)
    - [Rendering System](#rendering-system)
    - [Physics Scale Convention](#physics-scale-convention)
    - [Physics System](#physics-system)
    - [Serialization System](#serialization-system)
    - [Code Organization](#code-organization)
    - [Core Systems](#core-systems)
    - [Math Utilities](#math-utilities)
  - [Dependencies](#dependencies)
  - [Building the Project](#building-the-project)
    - [Docker Build (Recommended)](#docker-build-recommended)
    - [Build Options](#build-options)
    - [Examples](#examples)
    - [Build Output](#build-output)
    - [Building Example Project](#building-example-project)
    - [Dependencies](#dependencies-1)
    - [Simplified Include Paths](#simplified-include-paths)
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
  - `CRenderable`: Visual representation with support for sprites, shapes, and lines
  - `CMaterial`: Material properties including textures, shaders, tint, opacity, and blend modes
  - `CTexture`: Texture asset management and loading
  - `CShader`: Shader program management (vertex and fragment shaders)
  - `CName`: Provides naming functionality for entities
  - `CInputController`: Entity-specific input handling with action bindings
  - `CAudioListener`: Marks entity as audio listener for spatial audio
  - `CAudioSource`: Enables audio playback on entities (2D or spatial)

### Rendering System
- **SRenderer**: ECS-based rendering pipeline with SFML backend
  - Automatic rendering of all entities with `CRenderable` components
  - Z-index based layering for draw order control
  - Camera system for view transformations
  - **Visual Types**:
    - Sprites with texture support
    - Primitive shapes (rectangles, circles)
    - Lines with configurable thickness
    - Custom rendering support
  - **Material System**:
    - Texture mapping with `CTexture` components
    - Shader support via `CShader` components
    - Color tinting and opacity control
    - Blend mode options (Alpha, Additive, Multiply, None)
  - **Coordinate System Integration**:
    - Automatic conversion between physics (meters, Y-up) and screen space (pixels, Y-down)
    - Proper rotation and scale transformations
    - Multi-polygon collider bounds calculation for sprite scaling
  - Clear separation between game logic and rendering

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
The codebase is organized using pragma regions for better readability with sections for Variables, Methods, and Templates.

### Core Systems
- **Renderer (SRenderer)**: ECS-based rendering system with automatic entity rendering
  - Z-index layering and camera transformations
  - Sprite, shape, and line primitive support
  - Material system with textures, shaders, and blend modes
  - Access via `gameEngine.getRenderer()`
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

**Build Commands:**
- Linux build (with tests): `docker-compose exec dev ./build_linux.sh`
- Linux build (without tests): `docker-compose exec dev ./build_linux.sh --no-tests`
- Clean build: `docker-compose exec dev ./build_linux.sh --clean`
- Windows cross-compilation: `docker-compose exec dev ./build_windows.sh`
- Enter development environment: `docker-compose exec dev /bin/bash`


### Build Options

The build script (`build.sh`) provides several options:

- `-h, --help`: Show help message
- `-t, --type TYPE`: Set build type (Debug/Release) [default: Debug]
- `-s, --shared`: Build as shared library [default: OFF]
- `--no-tests`: Skip building and running tests
- `-c, --clean`: Clean build directory
- `-i, --install-prefix`: Set install prefix [default: ./package]

### Examples

- Clean build with tests: `./build.sh --clean`
- Release build without tests: `./build.sh -t Release --no-tests`
- Build as shared library: `./build.sh -s`


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

There is a build script in the example project that will build the GameEngine package and copy it to the example project. Run `./build_example.sh` from the example_project directory.

**NOTE: YOU MUST RUN THE BUILD.SH SCRIPT IN THE ROOT DIRECTORY FIRST.**

### Dependencies
The build script automatically handles the following dependencies:
- SFML (Graphics, Window, System)
- Dear ImGui
- ImGui-SFML

Dependencies are dynamically linked by default. The shared libraries will be included in the package's bin directory.
You will be required to link the dependencies manually in your project.
The GameEngine class provides the public API for accessing all engine systems and managers:
- `getRenderer()` - Rendering system with camera and material support
- `getEntityManager()` - Entity lifecycle and component management
- `getPhysics()` - Box2D physics system
- `getAudioSystem()` - Audio playback and control
- `getInputManager()` - Keyboard and mouse input
- `getSceneManager()` - Scene loading and saving
- `getComponentFactory()` - Component creation management
- `getPhysics()` - Box2D physics system
- `getAudioSystem()` - Audio playback and control
- `getInputManager()` - Keyboard and mouse input
- `getSceneManager()` - Scene loading and saving
- `getComponentFactory()` - Component creation

### Simplified Include Paths

The engine uses a flat include structure. Headers can be included directly without subdirectory prefixes (e.g., `#include <CTransform.h>` instead of `#include <components/CTransform.h>`).

## Project Structure

The project is organized with:
- `include/` - Public headers for entities, components, systems, and utilities
- `src/` - Implementation source files
- `tests/` - Unit tests
- `Example/` - Example game project
- `build_tools/` - Build scripts for different platforms

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