# 2D Game Engine

## What Is This?
- An attempt to make a very rudimentary and simple 2D game engine using libraries such as SMFL for rendering and custom Physics.
- To experiment with the Entity Component System (ECS) architectural pattern.

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