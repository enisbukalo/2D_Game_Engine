# Particle System Documentation

## Overview

The particle system (`SParticle`) provides efficient particle rendering and simulation for visual effects like smoke, fire, water splashes, bubble trails, and more. It uses SFML's vertex arrays for batched rendering and follows the ECS architecture with `CParticleEmitter` components.

## Features

- **ECS Integration**: Particle emitters are components (`CParticleEmitter`) attached to entities
- **Efficient Rendering**: Particles rendered using SFML vertex arrays per emitter
- **Physics-Based Motion**: Particles support velocity, acceleration, and gravity
- **Multiple Emission Shapes**:
  - Point: Single point emission (default)
  - Circle: Emit from circle edge
  - Rectangle: Emit from rectangle edges
  - Line: Emit along a line segment
  - Polygon: Emit from polygon edges (with convex hull support)
- **Customizable Properties**:
  - Color interpolation over lifetime
  - Alpha fading
  - Size scaling with configurable end scale
  - Rotation and rotation speed
  - Emission patterns (continuous, directional spread)
  - Z-index for render ordering
  - Position offset from entity
  - Emit outward from shape edges
- **Automatic Lifecycle Management**: Particles are automatically removed when they expire

## Architecture

### Components

1. **Particle** - Individual particle data structure containing:
   - Position, velocity, acceleration
   - Color and alpha
   - Size, initialSize, rotation, rotation speed
   - Lifetime and age
   - Alive state

2. **CParticleEmitter** - ECS component attached to entities:
   - Emission direction and spread angle
   - Speed, lifetime, size ranges (min/max)
   - Emission rate (particles per second)
   - Color interpolation (start/end color)
   - Alpha fade (start/end alpha)
   - Gravity/acceleration
   - Visual effects (fade out, shrink with configurable end scale)
   - Maximum particle count
   - Position offset from entity
   - Z-index for render ordering
   - Emission shape (Point, Circle, Rectangle, Line, Polygon)
   - Shape-specific parameters (radius, size, line endpoints, polygon vertices)
   - Emit outward option for shape-based emission
   - Optional texture for particles

3. **SParticle** - Singleton system that:
   - Updates all entities with CParticleEmitter components
   - Renders particles per emitter with vertex arrays
   - Handles coordinate conversion (world to screen)

## Usage

### Basic Setup

```cpp
// Access particle system through GameEngine
GameEngine engine(windowConfig, gravity);

// Initialize particle system (typically in your game's init)
auto& particleSystem = SParticle::instance();
particleSystem.initialize(&window, PIXELS_PER_METER);

// Update particle system each frame
particleSystem.update(deltaTime);

// Render particles for each entity with CParticleEmitter
// (typically done through SRenderer which handles z-ordering)
```

### Creating a Bubble Trail Effect

```cpp
// Create an entity with a particle emitter component
auto& entityManager = SEntity::instance();
auto* bubbleEntity = entityManager.addEntity("BubbleTrail");

// Add required components
auto* transform = bubbleEntity->addComponent<CTransform>();
transform->setPosition(Vec2(10.0f, 5.0f));

auto* emitter = bubbleEntity->addComponent<CParticleEmitter>();

// Configure bubble trail
emitter->setDirection(Vec2(0, -1));      // Emit downward
emitter->setSpreadAngle(0.8f);           // Wide spread
emitter->setMinSpeed(0.05f);
emitter->setMaxSpeed(0.2f);
emitter->setMinLifetime(0.8f);
emitter->setMaxLifetime(2.0f);
emitter->setMinSize(0.015f);
emitter->setMaxSize(0.04f);
emitter->setStartColor(Color(200, 230, 255));  // Light blue
emitter->setEndColor(Color(150, 200, 240));    // Darker blue
emitter->setStartAlpha(0.7f);
emitter->setEndAlpha(0.0f);                    // Fade out
emitter->setGravity(Vec2(0, 0.1f));            // Float upward
emitter->setFadeOut(true);
emitter->setShrink(true);
emitter->setShrinkEndScale(0.1f);              // Shrink to 10% of original size
emitter->setMaxParticles(300);
emitter->setEmissionRate(20.0f);               // 20 particles per second

// The emitter automatically follows the entity's transform
// Just update the entity's position to move the emitter:
transform->setPosition(newPosition);

// Use position offset for fine-tuned placement
emitter->setPositionOffset(Vec2(0.0f, -0.5f));  // Offset behind the entity
```

### Creating Other Effects

#### Smoke Trail
```cpp
auto* emitter = entity->addComponent<CParticleEmitter>();
emitter->setDirection(Vec2(0, 1));     // Rise upward
emitter->setSpreadAngle(0.5f);
emitter->setMinSpeed(0.2f);
emitter->setMaxSpeed(0.5f);
emitter->setMinLifetime(2.0f);
emitter->setMaxLifetime(4.0f);
emitter->setMinSize(0.05f);
emitter->setMaxSize(0.15f);
emitter->setStartColor(Color(80, 80, 80));    // Dark gray
emitter->setEndColor(Color(120, 120, 120));   // Light gray
emitter->setStartAlpha(0.8f);
emitter->setEndAlpha(0.0f);
emitter->setGravity(Vec2(0, -0.05f));  // Slight upward drift
emitter->setEmissionRate(20.0f);       // 20 particles/second
emitter->setFadeOut(true);
emitter->setShrink(false);             // Don't shrink smoke
```

#### Fire/Sparks
```cpp
auto* emitter = entity->addComponent<CParticleEmitter>();
emitter->setDirection(Vec2(0, 1));
emitter->setSpreadAngle(0.3f);
emitter->setMinSpeed(0.5f);
emitter->setMaxSpeed(1.0f);
emitter->setMinLifetime(0.3f);
emitter->setMaxLifetime(0.8f);
emitter->setMinSize(0.02f);
emitter->setMaxSize(0.06f);
emitter->setStartColor(Color(255, 200, 0));   // Yellow-orange
emitter->setEndColor(Color(255, 50, 0));      // Red
emitter->setStartAlpha(1.0f);
emitter->setEndAlpha(0.0f);
emitter->setGravity(Vec2(0, 0.3f));    // Slight upward acceleration
emitter->setEmissionRate(50.0f);
emitter->setFadeOut(true);
emitter->setShrink(true);
emitter->setShrinkEndScale(0.05f);
```

#### Circle Emission (Ring Effect)
```cpp
auto* emitter = entity->addComponent<CParticleEmitter>();
emitter->setEmissionShape(EmissionShape::Circle);
emitter->setShapeRadius(1.0f);         // 1 meter radius
emitter->setEmitOutward(true);         // Particles emit outward from edge
emitter->setSpreadAngle(0.2f);
emitter->setMinSpeed(0.5f);
emitter->setMaxSpeed(1.0f);
emitter->setStartColor(Color::Cyan);
emitter->setEndColor(Color::Blue);
emitter->setEmissionRate(30.0f);
emitter->setFadeOut(true);
```

#### Line Emission (Trail/Wake Effect)
```cpp
auto* emitter = entity->addComponent<CParticleEmitter>();
emitter->setEmissionShape(EmissionShape::Line);
emitter->setLineStart(Vec2(-1.0f, 0.0f));   // Line from left
emitter->setLineEnd(Vec2(1.0f, 0.0f));      // to right (2 meters)
emitter->setDirection(Vec2(0, -1));          // Emit downward
emitter->setEmissionRate(40.0f);
```

#### Polygon Emission (Complex Shape)
```cpp
auto* emitter = entity->addComponent<CParticleEmitter>();
emitter->setEmissionShape(EmissionShape::Polygon);

// Define custom polygon vertices
std::vector<Vec2> vertices = {
    Vec2(-0.5f, -0.5f),
    Vec2(0.5f, -0.5f),
    Vec2(0.5f, 0.5f),
    Vec2(-0.5f, 0.5f)
};
emitter->setPolygonVertices(vertices);

// Or compute from collider vertices using convex hull
emitter->setPolygonFromConvexHull(colliderVertices);

emitter->setEmitOutward(true);  // Emit away from polygon center
emitter->setEmissionRate(25.0f);
```

## API Reference

### SParticle

#### Initialization
```cpp
static SParticle& instance();  // Singleton access
bool initialize(sf::RenderWindow* window, float pixelsPerMeter = 100.0f);
void shutdown();
bool isInitialized() const;
```

#### Update and Render
```cpp
void update(float deltaTime);                              // Updates all emitters
void renderEmitter(Entity* entity, sf::RenderWindow* window);  // Render single emitter
```

### CParticleEmitter Component

#### Emission Configuration
```cpp
void setDirection(const Vec2& dir);         // Base emission direction
void setSpreadAngle(float angle);           // Spread cone in radians
void setMinSpeed(float speed);              // Min initial speed (m/s)
void setMaxSpeed(float speed);              // Max initial speed (m/s)
void setMinLifetime(float lifetime);        // Min particle lifetime (s)
void setMaxLifetime(float lifetime);        // Max particle lifetime (s)
void setMinSize(float size);                // Min particle size (meters)
void setMaxSize(float size);                // Max particle size (meters)
void setEmissionRate(float rate);           // Particles per second
void setMaxParticles(int max);              // Maximum particle count
```

#### Visual Properties
```cpp
void setStartColor(const Color& color);     // Starting color
void setEndColor(const Color& color);       // Ending color (interpolated)
void setStartAlpha(float alpha);            // Starting alpha (0.0-1.0)
void setEndAlpha(float alpha);              // Ending alpha
void setFadeOut(bool fade);                 // Enable alpha fading
void setShrink(bool shrink);                // Enable size shrinking
void setShrinkEndScale(float scale);        // Final size scale (0.0-1.0)
void setMinRotationSpeed(float speed);      // Min rotation speed (rad/s)
void setMaxRotationSpeed(float speed);      // Max rotation speed (rad/s)
void setTexture(sf::Texture* tex);          // Optional particle texture
void setZIndex(int zIndex);                 // Render layer ordering
```

#### Physics Properties
```cpp
void setGravity(const Vec2& grav);          // Constant acceleration (m/s²)
void setPositionOffset(const Vec2& offset); // Offset from entity position
```

#### Emission Shapes
```cpp
void setEmissionShape(EmissionShape shape); // Point, Circle, Rectangle, Line, Polygon
void setShapeRadius(float radius);          // Circle radius (meters)
void setShapeSize(const Vec2& size);        // Rectangle size (meters)
void setLineStart(const Vec2& start);       // Line start point
void setLineEnd(const Vec2& end);           // Line end point
void setPolygonVertices(const std::vector<Vec2>& vertices);  // Custom polygon
void setPolygonFromConvexHull(const std::vector<Vec2>& vertices);  // Compute hull
void setEmitFromEdge(bool edge);            // Emit from edge vs filled
void setEmitOutward(bool outward);          // Emit outward from shape center
```

#### State Access
```cpp
size_t getAliveCount() const;               // Number of alive particles
std::vector<Particle>& getParticles();      // Access particle data
bool isActive() const;                      // Is component active?
void setActive(bool active);                // Enable/disable emission
```

## Performance Considerations

1. **Particle Limits**: Set appropriate `maxParticles` limits for each emitter to prevent performance degradation
2. **Per-Emitter Rendering**: Particles are rendered per emitter using vertex arrays
3. **Automatic Cleanup**: Dead particles are reused rather than removed, minimizing allocations
4. **Alpha Blending**: Uses alpha blending by default (sf::BlendAlpha)
5. **Particle Pooling**: Dead particles are recycled for new emissions

## Example Integration

See `Example/src/main.cpp` for a complete example of integrating the particle system. The example demonstrates:
- Initializing the particle system with window reference
- Adding CParticleEmitter components to entities
- Configuring emission properties
- Emitter follows entity transform automatically
- Z-index based render ordering

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
5. **Cleanup**: Remove or deactivate emitter entities when changing scenes
6. **Dynamic Emission**: Control emission rate based on game events (speed, damage, etc.)
7. **Emission Shapes**: Use appropriate shapes for realistic effects:
   - Circle for explosions and radial effects
   - Line for trails and wakes
   - Rectangle for area effects
   - Polygon for custom shapes matching colliders
8. **Emit Outward**: Enable `setEmitOutward(true)` for shape-based emitters to create expanding effects
9. **Z-Index**: Use `setZIndex()` to control particle layer relative to other renderables
10. **Position Offset**: Use `setPositionOffset()` for fine-tuned emitter placement relative to entity
