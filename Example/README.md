# Bounce Game Example

This is a simple example game demonstrating the use of the GameEngine library.

## Features

- 2 balls with gravity that bounce off the floor and walls
- Physics simulation with restitution (bounciness)
- Simple rendering with SFML

## Building

### Prerequisites
- Docker container with MinGW cross-compilation setup
- GameEngine library (in `GameEngine-Windows-Debug` directory)

### Build Instructions

Build the example inside the Docker container:

```bash
# From the project root, enter the container
docker-compose exec dev bash

# Navigate to Example_Game and run the build script
cd Example_Game
chmod +x build_example.sh
./build_example.sh
```

Or run it directly without entering the container:

```bash
docker-compose exec dev bash -c "cd Example_Game && dos2unix build_example.sh && chmod +x build_example.sh && ./build_example.sh"
```

### Running

The executable will be in `Example_Game/build/bin/BounceGame.exe`

Run it on Windows (outside the container):

```bash
cd Example_Game/build/bin
./BounceGame.exe
```

## Controls

- Press `ESC` to quit
- Close the window to exit

## Description

The game creates:
- A gray floor at the bottom of the window
- A red ball starting at position (200, 100) with horizontal velocity
- A blue ball starting at position (600, 150) with initial velocity

Both balls fall under gravity and bounce off the floor and walls with a restitution coefficient of 0.8.
