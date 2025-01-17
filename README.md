# 2D Game Engine

A modern C++ 2D game engine built with SFML, featuring an Entity Component System (ECS) architecture.

## Features

### Entity Component System (ECS)
- **Entity Management**: Flexible entity creation and lifecycle management
- **Component-Based Architecture**: Modular component system for easy extension
- **Built-in Components**:
  - `CTransform`: Handles position, velocity, scale, and rotation
  - `CGravity`: Implements basic gravity physics
  - `CName`: Provides naming functionality for entities

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
- **Component Factory**: Provides a factory pattern for component creation
- **Serialization**: JSON-based serialization for game state persistence

### Math Utilities
- **Vec2**: A 2D vector class with common operations:
  - Vector arithmetic (add, subtract, multiply, divide)
  - Normalization
  - Rotation
  - Distance calculations

## Dependencies
- SFML 2.6.x: Graphics and window management
- nlohmann_json 3.11.x: JSON serialization
- C++17 or later

## Building the Project

The project includes a build script (`build.sh`) that handles the build process, including dependency management, testing, and packaging.

### Build Script Usage

```bash
./build.sh [options]
```

#### Options:
- `-h, --help`: Show help message
- `-t, --type TYPE`: Build type (Debug|Release) [default: Debug]
- `-s, --shared`: Build as shared library [default: OFF]
- `--no-tests`: Disable building tests
- `-c, --clean`: Clean build directory before building
- `-i, --install PREFIX`: Install to specified prefix
- `-p, --package`: Create distributable package

### Examples

1. Basic debug build:
```bash
./build.sh
```

2. Release build with shared library:
```bash
./build.sh -t Release -s
```

3. Clean build with package creation:
```bash
./build.sh -c -p
```

4. Install to specific directory:
```bash
./build.sh -i "./install"
```

### Build Output

The build script will:
1. Configure the project with CMake
2. Build all dependencies (SFML, ImGui, etc.)
3. Build the main GameEngine library
4. Run tests (if enabled)
5. Create a package (if requested) with:
   - Headers in `package/include/`
   - Library files in `package/lib/`
   - Binary files in `package/bin/`

### Dependencies
The build script automatically handles the following dependencies:
- SFML (Graphics, Window, System)
- Dear ImGui
- ImGui-SFML
- nlohmann_json

All dependencies are statically linked into the final library unless the shared option is used.

## Usage Example
```cpp
// Create a game engine instance
GameEngine engine(window, Vec2(0, -9.81f), 1, 1.0f/60.0f);

// Create an entity
auto entity = entityManager.addEntity("player");

// Add components
auto transform = entity->addComponent<CTransform>();
auto gravity = entity->addComponent<CGravity>();

// Update game loop
while (engine.is_running()) {
    engine.readInputs();
    engine.update();
    engine.render();
}
```

## Project Structure
```
2D_Game_Engine/
├── include/          # Public headers
│   ├── Entity.h
│   ├── EntityManager.h
│   ├── ComponentFactory.h
│   └── GameEngine.h
├── components/       # Component implementations
│   ├── CTransform.h
│   ├── CGravity.h
│   └── CName.h
├── systems/         # System implementations
│   └── System.h
└── src/            # Source files
```

## Contributing
Contributions are welcome! Please feel free to submit pull requests.

## License
[Your chosen license]