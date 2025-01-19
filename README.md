# GameEngine

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
- Dear ImGui 1.91.x: Immediate mode GUI
- ImGui-SFML 2.6.x: SFML backend for Dear ImGui
- nlohmann_json 3.11.x: JSON serialization
- C++17 or later

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

### Dependencies
The build script automatically handles the following dependencies:
- SFML (Graphics, Window, System)
- Dear ImGui
- ImGui-SFML
- nlohmann_json

Dependencies are dynamically linked by default. The shared libraries will be included in the package's bin directory.
You will be required to link the dependencies manually in your project.

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
MIT License