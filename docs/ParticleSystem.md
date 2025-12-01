# Particle System Documentation

## Overview

The particle system (`SParticleSystem`) provides efficient particle rendering and simulation for visual effects like smoke, fire, water splashes, bubble trails, and more. It uses SFML's vertex arrays for batched rendering and supports multiple independent emitters.

## Features

- **Multiple Emitters**: Create and manage multiple particle emitters with different configurations
- **Efficient Rendering**: All particles rendered in a single draw call using vertex arrays
- **Physics-Based Motion**: Particles support velocity, acceleration, and gravity
- **Customizable Properties**:
  - Color interpolation over lifetime
  - Alpha fading
  - Size scaling
  - Rotation and rotation speed
  - Emission patterns (continuous, burst, directional spread)
- **Automatic Lifecycle Management**: Particles are automatically removed when they expire

## Architecture

### Components

1. **Particle** - Individual particle data structure containing:
   - Position, velocity, acceleration
   - Color and alpha
   - Size, rotation, rotation speed
   - Lifetime and age
   - Alive state

2. **ParticleEmitterConfig** - Configuration for particle emission:
   - Emitter position and direction
   - Speed, lifetime, size ranges (min/max)
   - Emission rate or burst count
   - Color interpolation (start/end)
   - Gravity/acceleration
   - Visual effects (fade, shrink)
   - Maximum particle count

3. **ParticleEmitter** - Emitter that spawns and manages particles:
   - Creates particles based on configuration
   - Updates particle physics
   - Manages particle lifecycle

4. **SParticleSystem** - Singleton system that:
   - Manages multiple emitters
   - Updates all emitters
   - Renders all particles efficiently

## Usage

### Basic Setup

```cpp
// Access particle system through GameEngine
GameEngine engine(windowConfig, gravity);

// Initialize particle system (typically in your game's init)
auto& particleSystem = engine.getParticleSystem();
particleSystem.initialize(PIXELS_PER_METER, SCREEN_HEIGHT);

// Update particle system each frame
particleSystem.update(deltaTime);

// Render particles (after scene but before UI)
particleSystem.render(window);
```

### Creating a Bubble Trail Effect

```cpp
// Access through GameEngine
auto& particleSystem = gameEngine.getParticleSystem();

// Configure bubble trail
ParticleEmitterConfig bubbleConfig;
bubbleConfig.position = Vec2(10, 5);  // Initial position
bubbleConfig.direction = Vec2(0, -1); // Emit downward
bubbleConfig.spreadAngle = 0.8f;      // Wide spread
bubbleConfig.minSpeed = 0.05f;
bubbleConfig.maxSpeed = 0.2f;
bubbleConfig.minLifetime = 0.8f;
bubbleConfig.maxLifetime = 2.0f;
bubbleConfig.minSize = 0.015f;
bubbleConfig.maxSize = 0.04f;
bubbleConfig.startColor = Color(200, 230, 255);  // Light blue
bubbleConfig.endColor = Color(150, 200, 240);    // Darker blue
bubbleConfig.startAlpha = 0.7f;
bubbleConfig.endAlpha = 0.0f;                    // Fade out
bubbleConfig.gravity = Vec2(0, 0.1f);            // Float upward
bubbleConfig.fadeOut = true;
bubbleConfig.shrink = true;
bubbleConfig.maxParticles = 300;

// Create emitter
ParticleEmitter* bubbleEmitter = particleSystem.createEmitter(bubbleConfig);

// Update emitter position each frame (for moving objects)
bubbleEmitter->setPosition(boatPosition);

// Emit particles manually based on object speed
if (objectIsMoving) {
    int particleCount = calculateParticleCount(speed);
    for (int i = 0; i < particleCount; ++i) {
        bubbleEmitter->emit();
    }
}
```

### Creating Other Effects

#### Smoke Trail
```cpp
ParticleEmitterConfig smokeConfig;
smokeConfig.direction = Vec2(0, 1);  // Rise upward
smokeConfig.spreadAngle = 0.5f;
smokeConfig.minSpeed = 0.2f;
smokeConfig.maxSpeed = 0.5f;
smokeConfig.minLifetime = 2.0f;
smokeConfig.maxLifetime = 4.0f;
smokeConfig.minSize = 0.05f;
smokeConfig.maxSize = 0.15f;
smokeConfig.startColor = Color(80, 80, 80);   // Dark gray
smokeConfig.endColor = Color(120, 120, 120);  // Light gray
smokeConfig.startAlpha = 0.8f;
smokeConfig.endAlpha = 0.0f;
smokeConfig.gravity = Vec2(0, -0.05f);  // Slight upward drift
smokeConfig.emissionRate = 20.0f;       // 20 particles/second
smokeConfig.fadeOut = true;
smokeConfig.shrink = false;             // Don't shrink smoke
```

#### Fire/Sparks
```cpp
ParticleEmitterConfig fireConfig;
fireConfig.direction = Vec2(0, 1);
fireConfig.spreadAngle = 0.3f;
fireConfig.minSpeed = 0.5f;
fireConfig.maxSpeed = 1.0f;
fireConfig.minLifetime = 0.3f;
fireConfig.maxLifetime = 0.8f;
fireConfig.minSize = 0.02f;
fireConfig.maxSize = 0.06f;
fireConfig.startColor = Color(255, 200, 0);  // Yellow-orange
fireConfig.endColor = Color(255, 50, 0);     // Red
fireConfig.startAlpha = 1.0f;
fireConfig.endAlpha = 0.0f;
fireConfig.gravity = Vec2(0, 0.3f);  // Slight upward acceleration
fireConfig.emissionRate = 50.0f;
fireConfig.fadeOut = true;
fireConfig.shrink = true;
```

#### Burst/Explosion Effect
```cpp
ParticleEmitterConfig burstConfig;
burstConfig.direction = Vec2(0, 0);   // No preferred direction
burstConfig.spreadAngle = 6.28f;      // Full 360 degrees
burstConfig.minSpeed = 1.0f;
burstConfig.maxSpeed = 3.0f;
burstConfig.minLifetime = 0.5f;
burstConfig.maxLifetime = 1.5f;
burstConfig.startColor = Color::White;
burstConfig.endColor = Color::Yellow;
burstConfig.emissionRate = 0.0f;      // No continuous emission
burstConfig.fadeOut = true;
burstConfig.shrink = true;

auto* burstEmitter = particleSystem.createEmitter(burstConfig);

// Trigger explosion
burstEmitter->emitBurst(100);  // Emit 100 particles at once
```

## API Reference

### SParticleSystem

#### Initialization
```cpp
bool initialize(float pixelsPerMeter = 100.0f, float screenHeight = 600.0f);
void shutdown();
```

#### Update and Render
```cpp
void update(float deltaTime);
void render(sf::RenderWindow* window);
```

#### Emitter Management
```cpp
ParticleEmitter* createEmitter(const ParticleEmitterConfig& config);
void removeEmitter(ParticleEmitter* emitter);
void clearEmitters();
size_t getEmitterCount() const;
size_t getTotalParticleCount() const;
```

### ParticleEmitter

#### Emission Control
```cpp
void emit();                    // Emit single particle
void emitBurst(int count);      // Emit multiple particles
void setActive(bool active);    // Enable/disable emission
bool isActive() const;
```

#### Position and Direction
```cpp
void setPosition(const Vec2& position);
Vec2 getPosition() const;
void setDirection(const Vec2& direction);
```

#### Configuration
```cpp
ParticleEmitterConfig& getConfig();
const ParticleEmitterConfig& getConfig() const;
```

#### Particle Management
```cpp
const std::vector<Particle>& getParticles() const;
size_t getAliveCount() const;
void clear();  // Remove all particles
```

## Performance Considerations

1. **Particle Limits**: Set appropriate `maxParticles` limits for each emitter to prevent performance degradation
2. **Batched Rendering**: All particles from all emitters are rendered in a single draw call
3. **Automatic Cleanup**: Dead particles are automatically removed to prevent memory growth
4. **Additive Blending**: Uses additive blending by default for glowing effects (can be customized)

## Example Integration

See `Example/src/main.cpp` for a complete example of integrating the particle system with a player boat's bubble trail effect. The example demonstrates:
- Initializing the particle system
- Creating an emitter
- Updating emitter position based on object movement
- Dynamic particle emission based on object speed
- Proper cleanup and restart handling

## Coordinate System

The particle system uses the same coordinate system as the physics engine:
- World space: meters (Y-up)
- Screen space: pixels (Y-down)
- Automatic conversion via `pixelsPerMeter` and `screenHeight` parameters

## Tips and Best Practices

1. **Performance**: Limit total particles across all emitters to ~1000-2000 for smooth 60 FPS
2. **Visual Quality**: Use color interpolation and alpha fading for smooth transitions
3. **Realism**: Add gravity/acceleration for physics-based motion
4. **Variety**: Randomize particle properties (size, speed, lifetime) for natural appearance
5. **Cleanup**: Call `clearEmitters()` when restarting or changing scenes
6. **Dynamic Emission**: Control emission rate or burst count based on game events (speed, damage, etc.)
