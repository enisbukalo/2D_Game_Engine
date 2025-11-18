# Physics Optimization Plan - Issue #44

**Issue:** Optimize Physics Performance With Static Object Skipping And Entity Sleeping
**Branch:** `44-optimize-physics-performance-with-static-object-skipping-and-entity-sleeping`
**Date:** 2025-11-18
**Assignee:** Enis Bukalo

---

## Executive Summary

This document outlines a comprehensive plan to optimize the 2D Game Engine physics system performance. The current implementation processes all entities equally, regardless of whether they're moving, static, or at rest. This results in unnecessary computation, especially with large numbers of physics objects.

**Expected Performance Improvements:**
- Debug output removal: ~5-10% improvement
- Static object skipping: ~10-20% improvement
- Entity sleeping: ~40-60% improvement (once entities settle)
- **Combined: 2-5x faster physics in typical game scenarios**

---

## Current Performance Issues

### Analysis Summary

Based on comprehensive code review of the physics system, the following issues were identified:

#### 1. Unconditional Debug Output (HIGH IMPACT)
- **36 `std::cout` statements** in `S2DPhysics.cpp` with no conditional compilation
- Debug output runs in **both Debug and Release builds**
- Console I/O is extremely slow and generates thousands of lines per second with 100+ entities
- No `#ifdef DEBUG` guards anywhere in the physics code
- CMake doesn't define DEBUG/NDEBUG macros for conditional compilation

**Affected Code Locations:**
- `S2DPhysics::update()` - Lines 25-33 (velocity output for ALL entities every frame)
- `S2DPhysics::updateQuadtree()` - Lines 59-78 (entity positions and insertions)
- `S2DPhysics::checkCollisions()` - Lines 148-216 (extensive collision logging)
- `S2DPhysics::handleCollision()` - Lines 224-228
- `S2DPhysics::resolveCircleVsCircle()` - Lines 305-308
- `S2DPhysics::resolveCircleVsBox()` - Lines 381-387
- `S2DPhysics::resolveBoxVsBox()` - Lines 458-461

#### 2. Static Objects Processed Unnecessarily (MEDIUM IMPACT)
- Static entities (floors, walls) have their positions recalculated every frame
- `integratePositions()` processes ALL entities including static ones
- `updateQuadtree()` rebuilds and re-inserts static entities every frame
- `isStatic()` check only used in collision resolution, not in physics updates

**Example:** With 100 balls + 1 static floor:
- Floor position is recalculated 60 times per second despite never moving
- Floor is removed from quadtree and re-inserted 60 times per second
- ~1% of objects are static but consume equal processing time as dynamic objects

#### 3. No Entity Sleeping System (HIGH IMPACT)
- Entities at rest (settled on floor) continue running full physics updates
- No mechanism to detect when objects have stopped moving
- In typical physics simulation, 60-80% of objects are at rest after a few seconds
- These resting objects consume CPU cycles unnecessarily

**Scenario:** After balls settle on the floor:
- All 100 balls continue updating velocity, position, collision checks
- Only ~20 balls are actually moving/bouncing
- 80 settled balls waste ~80% of physics computation

#### 4. Quadtree Rebuilt Every Frame (MEDIUM IMPACT - DEFERRED)
- Entire spatial structure cleared and rebuilt each frame
- All entities re-queried from EntityManager
- AABB bounds recalculated for every entity even if unchanged
- No incremental updates or caching

**Note:** This optimization is noted but deferred to future work.

---

## Implementation Plan

### Overview

The optimizations will be implemented in **3 sequential steps**, with each step tested before proceeding to the next. This ensures stability and allows us to measure the performance impact of each optimization individually.

---

## Step 1: Conditional Debug Output

**Time Estimate:** 15-20 minutes
**Difficulty:** Low
**Risk:** Very Low

### Goal
Remove debug output from Release builds while keeping it available in Debug builds for development.

### Implementation Details

#### 1.1 Update Root CMakeLists.txt

**File:** `CMakeLists.txt` (project root)

**Location:** After the `project()` declaration (around line 10)

**Add:**
```cmake
# Define DEBUG macro for Debug builds only
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()
```

**Alternative (more robust):**
```cmake
# Define DEBUG macro for Debug builds only (works with multi-config generators)
add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)
```

**Rationale:** The second approach uses CMake generator expressions and works correctly with multi-configuration generators (like Visual Studio) as well as single-configuration generators (like Unix Makefiles).

#### 1.2 Update Example_Game CMakeLists.txt

**File:** `Example_Game/CMakeLists.txt`

**Location:** After the `project()` declaration

**Add the same code as 1.1**

**Rationale:** Example_Game is a separate CMake project, so it needs its own DEBUG definition.

#### 1.3 Wrap Debug Output in S2DPhysics.cpp

**File:** `src/S2DPhysics.cpp`

**Changes Required:**

##### Location 1: update() function (Lines 24-34)
**Current:**
```cpp
void S2DPhysics::update(float deltaTime)
{
    handleGravity(deltaTime);
    integratePositions(deltaTime);
    updateQuadtree();
    checkCollisions();

    // Debug: print all entity velocities after physics update
    std::cout << "[DEBUG] S2DPhysics::update complete. Entity velocities:" << std::endl;
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 vel       = transform->getVelocity();
        std::cout << "[DEBUG]   " << entity->getTag() << ": velocity=(" << vel.x << "," << vel.y << ")" << std::endl;
    }
}
```

**Updated:**
```cpp
void S2DPhysics::update(float deltaTime)
{
    handleGravity(deltaTime);
    integratePositions(deltaTime);
    updateQuadtree();
    checkCollisions();

#ifdef DEBUG
    // Debug: print all entity velocities after physics update
    std::cout << "[DEBUG] S2DPhysics::update complete. Entity velocities:" << std::endl;
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 vel       = transform->getVelocity();
        std::cout << "[DEBUG]   " << entity->getTag() << ": velocity=(" << vel.x << "," << vel.y << ")" << std::endl;
    }
#endif
}
```

##### Location 2: updateQuadtree() function (Lines 59-96)
Wrap all `std::cout` statements in `#ifdef DEBUG ... #endif` blocks:
- Lines 59-61 (entity count and bounds)
- Lines 68-78 (per-entity insertion logging)

##### Location 3: checkCollisions() function (Lines 148-218)
Wrap all `std::cout` statements:
- Line 148 (collision count)
- Lines 153-157 (skip reasons)
- Lines 160-180 (entity bounds and quadtree results)
- Lines 185-198 (collision pair checks)
- Lines 207-216 (collision detection)

##### Location 4: handleCollision() function (Lines 224-228)
Wrap debug output for collider retrieval

##### Location 5: resolveCircleVsCircle() function (Lines 305-308)
Wrap collision resolution debug output

##### Location 6: resolveCircleVsBox() function (Lines 381-387)
Wrap collision resolution debug output

##### Location 7: resolveBoxVsBox() function (Lines 458-461)
Wrap collision resolution debug output

### Testing Step 1

**After implementing Step 1:**

1. **Build Debug configuration:**
   ```bash
   docker-compose exec dev ./build_tools/build.sh
   ```
   - Verify debug output appears in console
   - Verify physics works correctly

2. **Build Release configuration:**
   ```bash
   docker-compose exec dev cmake -DCMAKE_BUILD_TYPE=Release -B build-release
   docker-compose exec dev cmake --build build-release
   ```
   - Verify NO debug output appears
   - Verify physics still works correctly

3. **Run Example_Game:**
   ```bash
   docker-compose exec dev ./Example_Game/build_example.sh
   ```
   - Build with 100 balls
   - Measure perceived performance improvement
   - Console should be silent in release builds

4. **Run Unit Tests:**
   ```bash
   docker-compose exec dev ./build/tests/GameEngine_Tests
   ```
   - All 145+ tests should pass
   - No test should depend on debug output

**Expected Results:**
- Release build runs faster due to no console I/O
- Debug build still has full logging for development
- No behavioral changes to physics

**Success Criteria:**
- ✅ All tests pass
- ✅ No console output in Release builds
- ✅ Debug output present in Debug builds
- ✅ Physics behavior unchanged

---

## Step 2: Skip Static Objects

**Time Estimate:** 20-30 minutes
**Difficulty:** Medium
**Risk:** Low

### Goal
Bypass physics updates (position integration, gravity) for entities marked as static.

### Implementation Details

#### 2.1 Update integratePositions()

**File:** `src/S2DPhysics.cpp`

**Location:** Lines 99-117

**Current Code:**
```cpp
void S2DPhysics::integratePositions(float deltaTime)
{
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        if (!transform)
            continue;

        // Update position based on velocity: position += velocity * deltaTime
        Vec2 currentPos = transform->getPosition();
        Vec2 velocity   = transform->getVelocity();
        Vec2 newPos     = currentPos + (velocity * deltaTime);

        transform->setPosition(newPos);
    }
}
```

**Updated Code:**
```cpp
void S2DPhysics::integratePositions(float deltaTime)
{
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        if (!transform)
            continue;

        // Skip static objects - they don't move
        auto collider = entity->getComponentDerived<CCollider>();
        if (collider && collider->isStatic())
            continue;

        // Update position based on velocity: position += velocity * deltaTime
        Vec2 currentPos = transform->getPosition();
        Vec2 velocity   = transform->getVelocity();
        Vec2 newPos     = currentPos + (velocity * deltaTime);

        transform->setPosition(newPos);
    }
}
```

**Rationale:**
- Static objects have zero velocity and never move
- Skipping position calculation saves Vec2 operations and setter call
- Early-out pattern is efficient

#### 2.2 Update handleGravity()

**File:** `src/S2DPhysics.cpp`

**Location:** Lines 119-140

**Current Code:**
```cpp
void S2DPhysics::handleGravity(float deltaTime)
{
    auto& entityManager = EntityManager::instance();

    // Apply gravity force to velocities
    auto entities = entityManager.getEntitiesWithComponent<CGravity>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        auto gravity   = entity->getComponent<CGravity>();

        if (transform && gravity && gravity->isActive())
        {
            // Apply gravity to velocity: v = v0 + a*dt
            Vec2 currentVelocity = transform->getVelocity();
            Vec2 force           = gravity->getForce();
            Vec2 newVelocity     = currentVelocity + (force * deltaTime);

            transform->setVelocity(newVelocity);
        }
    }
}
```

**Updated Code:**
```cpp
void S2DPhysics::handleGravity(float deltaTime)
{
    auto& entityManager = EntityManager::instance();

    // Apply gravity force to velocities
    auto entities = entityManager.getEntitiesWithComponent<CGravity>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        auto gravity   = entity->getComponent<CGravity>();

        if (transform && gravity && gravity->isActive())
        {
            // Skip static objects - gravity doesn't affect immovable objects
            auto collider = entity->getComponentDerived<CCollider>();
            if (collider && collider->isStatic())
                continue;

            // Apply gravity to velocity: v = v0 + a*dt
            Vec2 currentVelocity = transform->getVelocity();
            Vec2 force           = gravity->getForce();
            Vec2 newVelocity     = currentVelocity + (force * deltaTime);

            transform->setVelocity(newVelocity);
        }
    }
}
```

**Rationale:**
- Static objects shouldn't have gravity applied (they're immovable)
- This is a safety check preventing bugs (e.g., accidentally adding CGravity to floor)
- Consistent with physics simulation principles

#### 2.3 Add Comment for Future Optimization

**File:** `src/S2DPhysics.cpp`

**Location:** In `updateQuadtree()` function, around line 54

**Add Comment:**
```cpp
void S2DPhysics::updateQuadtree()
{
    // TODO: Optimization opportunity - Separate static and dynamic quadtrees
    // Static objects never move, so they could be in a separate tree that's only
    // built once. This would reduce the number of entities processed here by ~10-20%.
    // See Issue #44 for more details.

    m_quadtree->clear();

    // ... rest of function
}
```

**Rationale:**
- Documents optimization opportunity for future work
- Links to GitHub issue for context
- Doesn't add complexity now but preserves knowledge

### Testing Step 2

**After implementing Step 2:**

1. **Build and run unit tests:**
   ```bash
   docker-compose exec dev ./build_tools/build.sh
   docker-compose exec dev ./build/tests/GameEngine_Tests
   ```
   - All collision tests should pass
   - Static vs dynamic collision tests are critical

2. **Test with Example_Game:**
   ```bash
   docker-compose exec dev ./Example_Game/build_example.sh
   ```
   - Run with 100 balls
   - Verify floor remains stationary
   - Verify balls still bounce correctly off floor
   - Measure frame rate improvement

3. **Edge case testing:**
   - Create entity with static collider + CGravity component
   - Verify gravity is NOT applied (safety check works)
   - Verify position doesn't change

**Expected Results:**
- Static floor is skipped in physics updates
- ~10-20% performance improvement (1 static object out of 101 total)
- No change in physics behavior

**Success Criteria:**
- ✅ All tests pass
- ✅ Static objects don't move
- ✅ Dynamic objects still interact with static objects correctly
- ✅ Performance improvement measurable

---

## Step 3: Entity Sleeping System

**Time Estimate:** 60-90 minutes
**Difficulty:** High
**Risk:** Medium

### Goal
Implement sleeping state for entities at rest to skip physics updates for settled objects.

### Overview

The sleeping system works as follows:
1. **Sleep Detection:** Monitor entity velocity; if below threshold for X seconds → sleep
2. **Sleep State:** Sleeping entities skip gravity, integration, but remain in quadtree
3. **Wake Conditions:** Wake on collision with active object or manual velocity change
4. **Performance:** In settled state (balls resting on floor), 80%+ of entities can sleep

### Implementation Details

#### 3.1 Extend CTransform Component

**File:** `include/components/CTransform.h`

**Current Structure (abbreviated):**
```cpp
class CTransform : public Component
{
private:
    Vec2  m_position;
    Vec2  m_velocity;
    Vec2  m_scale;
    float m_rotation;

public:
    Vec2  getPosition() const { return m_position; }
    Vec2  getVelocity() const { return m_velocity; }
    void  setPosition(const Vec2& position) { m_position = position; }
    void  setVelocity(const Vec2& velocity) { m_velocity = velocity; }
    // ... other methods
};
```

**Updated Structure:**
```cpp
class CTransform : public Component
{
private:
    Vec2  m_position;
    Vec2  m_velocity;
    Vec2  m_scale;
    float m_rotation;

    // Sleeping state
    bool  m_isSleeping;        ///< Is this entity currently asleep?
    float m_sleepTimer;        ///< Time entity has been nearly motionless (in seconds)
    Vec2  m_previousVelocity;  ///< Velocity from previous frame for sleep detection

public:
    // Existing methods...
    Vec2  getPosition() const { return m_position; }
    Vec2  getVelocity() const { return m_velocity; }
    void  setPosition(const Vec2& position) { m_position = position; }
    void  setVelocity(const Vec2& velocity);  // Modified to wake on velocity change

    // Sleep state methods
    bool  isSleeping() const { return m_isSleeping; }
    void  setSleeping(bool sleeping) { m_isSleeping = sleeping; }
    void  wake() { m_isSleeping = false; m_sleepTimer = 0.0f; }
    float getSleepTimer() const { return m_sleepTimer; }
    void  incrementSleepTimer(float deltaTime) { m_sleepTimer += deltaTime; }
    void  resetSleepTimer() { m_sleepTimer = 0.0f; }
    Vec2  getPreviousVelocity() const { return m_previousVelocity; }
    void  setPreviousVelocity(const Vec2& vel) { m_previousVelocity = vel; }
};
```

**Rationale for Each Field:**
- `m_isSleeping`: Boolean flag - fast to check, no overhead when false
- `m_sleepTimer`: Accumulated time below threshold - prevents rapid sleep/wake oscillation
- `m_previousVelocity`: For detecting velocity changes - used in sleep state updates

#### 3.2 Implement CTransform Sleep Methods

**File:** `src/CTransform.cpp`

**Add Constructor Initialization:**
```cpp
CTransform::CTransform(Vec2 position, Vec2 scale, float rotation)
    : m_position(position)
    , m_velocity(0.0f, 0.0f)
    , m_scale(scale)
    , m_rotation(rotation)
    , m_isSleeping(false)      // Start awake
    , m_sleepTimer(0.0f)       // No sleep time accumulated
    , m_previousVelocity(0.0f, 0.0f)
{
}
```

**Modify setVelocity() to Wake:**
```cpp
void CTransform::setVelocity(const Vec2& velocity)
{
    m_velocity = velocity;

    // Wake entity if velocity is being manually set
    // This handles external forces or game logic changing velocity
    if (m_isSleeping && velocity.lengthSquared() > 0.0f)
    {
        wake();
    }
}
```

**Rationale:**
- Auto-wake on velocity change prevents sleeping entities from ignoring game logic
- Uses `lengthSquared()` to avoid sqrt for performance
- Only wakes if velocity is non-zero (setting to zero is fine)

#### 3.3 Update S2DPhysics Header

**File:** `include/systems/S2DPhysics.h`

**Add Private Method Declaration:**
```cpp
class S2DPhysics : public System
{
public:
    // ... existing methods
    void update(float deltaTime) override;

private:
    // ... existing methods
    void handleGravity(float deltaTime);
    void integratePositions(float deltaTime);
    void updateQuadtree();
    void checkCollisions();
    void handleCollision(Entity* a, Entity* b);

    // NEW: Sleep state management
    void updateSleepState(float deltaTime);

    // ... existing members
};
```

#### 3.4 Implement updateSleepState()

**File:** `src/S2DPhysics.cpp`

**Add New Function (after checkCollisions()):**
```cpp
void S2DPhysics::updateSleepState(float deltaTime)
{
    // Sleep thresholds
    constexpr float SLEEP_VELOCITY_THRESHOLD = 5.0f;    // Units per second
    constexpr float SLEEP_TIME_THRESHOLD     = 0.5f;    // Seconds of low velocity
    constexpr float WAKE_VELOCITY_THRESHOLD  = 10.0f;   // Wake if velocity exceeds this

    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        if (!transform)
            continue;

        // Skip static objects - they're always effectively "asleep"
        auto collider = entity->getComponentDerived<CCollider>();
        if (collider && collider->isStatic())
            continue;

        Vec2  velocity      = transform->getVelocity();
        float speedSquared  = velocity.lengthSquared();
        float speed         = std::sqrt(speedSquared);

        if (transform->isSleeping())
        {
            // Entity is asleep - check if it should wake
            if (speed > WAKE_VELOCITY_THRESHOLD)
            {
#ifdef DEBUG
                std::cout << "[DEBUG] Waking entity " << entity->getTag()
                          << " due to high velocity: " << speed << std::endl;
#endif
                transform->wake();
            }
        }
        else
        {
            // Entity is awake - check if it should sleep
            if (speed < SLEEP_VELOCITY_THRESHOLD)
            {
                // Low velocity - increment sleep timer
                transform->incrementSleepTimer(deltaTime);

                if (transform->getSleepTimer() >= SLEEP_TIME_THRESHOLD)
                {
#ifdef DEBUG
                    std::cout << "[DEBUG] Putting entity " << entity->getTag()
                              << " to sleep (velocity: " << speed << ")" << std::endl;
#endif
                    transform->setSleeping(true);
                }
            }
            else
            {
                // High velocity - reset sleep timer
                transform->resetSleepTimer();
            }
        }

        // Store velocity for next frame (if needed for future optimizations)
        transform->setPreviousVelocity(velocity);
    }
}
```

**Threshold Explanation:**
- `SLEEP_VELOCITY_THRESHOLD = 5.0f`: Below this speed for 0.5s → sleep
  - Chosen to be low enough to catch settled objects
  - High enough to avoid sleeping during slow rolling
- `SLEEP_TIME_THRESHOLD = 0.5f`: Must be slow for half a second
  - Prevents sleep/wake oscillation
  - Allows objects to settle naturally
- `WAKE_VELOCITY_THRESHOLD = 10.0f`: Wake if speed exceeds this
  - Higher than sleep threshold (hysteresis)
  - Prevents rapid sleep/wake cycling

**Note:** These thresholds may need tuning based on your game's physics scale and gravity values.

#### 3.5 Skip Sleeping Entities in handleGravity()

**File:** `src/S2DPhysics.cpp`

**Updated handleGravity():**
```cpp
void S2DPhysics::handleGravity(float deltaTime)
{
    auto& entityManager = EntityManager::instance();

    // Apply gravity force to velocities
    auto entities = entityManager.getEntitiesWithComponent<CGravity>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        auto gravity   = entity->getComponent<CGravity>();

        if (transform && gravity && gravity->isActive())
        {
            // Skip static objects - gravity doesn't affect immovable objects
            auto collider = entity->getComponentDerived<CCollider>();
            if (collider && collider->isStatic())
                continue;

            // NEW: Skip sleeping entities - they're at rest
            if (transform->isSleeping())
                continue;

            // Apply gravity to velocity: v = v0 + a*dt
            Vec2 currentVelocity = transform->getVelocity();
            Vec2 force           = gravity->getForce();
            Vec2 newVelocity     = currentVelocity + (force * deltaTime);

            transform->setVelocity(newVelocity);
        }
    }
}
```

#### 3.6 Skip Sleeping Entities in integratePositions()

**File:** `src/S2DPhysics.cpp`

**Updated integratePositions():**
```cpp
void S2DPhysics::integratePositions(float deltaTime)
{
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();

    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        if (!transform)
            continue;

        // Skip static objects - they don't move
        auto collider = entity->getComponentDerived<CCollider>();
        if (collider && collider->isStatic())
            continue;

        // NEW: Skip sleeping entities - they're at rest
        if (transform->isSleeping())
            continue;

        // Update position based on velocity: position += velocity * deltaTime
        Vec2 currentPos = transform->getPosition();
        Vec2 velocity   = transform->getVelocity();
        Vec2 newPos     = currentPos + (velocity * deltaTime);

        transform->setPosition(newPos);
    }
}
```

#### 3.7 Wake Entities on Collision

**File:** `src/S2DPhysics.cpp`

**Updated handleCollision():**
```cpp
void S2DPhysics::handleCollision(Entity* a, Entity* b)
{
#ifdef DEBUG
    std::cout << "[DEBUG] handleCollision: Getting colliders..." << std::endl;
#endif
    auto colliderA = a->getComponentDerived<CCollider>();
    auto colliderB = b->getComponentDerived<CCollider>();
#ifdef DEBUG
    std::cout << "[DEBUG] handleCollision: Got colliders (A=" << (colliderA != nullptr)
              << ", B=" << (colliderB != nullptr) << ")" << std::endl;
#endif

    // NEW: Wake both entities involved in collision
    if (auto transformA = a->getComponent<CTransform>())
    {
        if (transformA->isSleeping())
        {
#ifdef DEBUG
            std::cout << "[DEBUG] Waking entity " << a->getTag() << " due to collision" << std::endl;
#endif
            transformA->wake();
        }
    }
    if (auto transformB = b->getComponent<CTransform>())
    {
        if (transformB->isSleeping())
        {
#ifdef DEBUG
            std::cout << "[DEBUG] Waking entity " << b->getTag() << " due to collision" << std::endl;
#endif
            transformB->wake();
        }
    }

    // If either is a trigger, just notify
    if (colliderA->isTrigger() || colliderB->isTrigger())
    {
        // TODO: Emit collision event
        return;
    }

    // Otherwise, resolve the collision physically
    resolveCollision(a, b, colliderA, colliderB);
}
```

**Rationale:**
- Collision implies movement/force
- Both entities in collision should wake up
- Prevents sleeping entities from being stuck in collision state
- Wake happens BEFORE collision resolution

#### 3.8 Update Physics Loop

**File:** `src/S2DPhysics.cpp`

**Updated update():**
```cpp
void S2DPhysics::update(float deltaTime)
{
    handleGravity(deltaTime);      // Skips sleeping entities (Step 3.5)
    integratePositions(deltaTime); // Skips sleeping entities (Step 3.6)
    updateQuadtree();              // Still includes sleeping entities (for collision wake-up)
    checkCollisions();             // Wakes sleeping entities on collision (Step 3.7)
    updateSleepState(deltaTime);   // NEW: Put entities to sleep or wake them (Step 3.4)

#ifdef DEBUG
    // Debug: print all entity velocities after physics update
    std::cout << "[DEBUG] S2DPhysics::update complete. Entity velocities:" << std::endl;
    auto& entityManager = EntityManager::instance();
    auto  entities      = entityManager.getEntitiesWithComponent<CTransform>();
    for (auto* entity : entities)
    {
        auto transform = entity->getComponent<CTransform>();
        Vec2 vel       = transform->getVelocity();
        std::cout << "[DEBUG]   " << entity->getTag() << ": velocity=(" << vel.x << "," << vel.y << ")"
                  << (transform->isSleeping() ? " [SLEEPING]" : " [AWAKE]") << std::endl;
    }
#endif
}
```

**Order Matters:**
1. **handleGravity()** first - applies forces to awake entities
2. **integratePositions()** - updates positions based on velocity
3. **updateQuadtree()** - rebuilds spatial structure (includes sleeping entities)
4. **checkCollisions()** - detects collisions and wakes sleeping entities
5. **updateSleepState()** last - determines which entities should sleep based on final velocity

### Testing Step 3

**After implementing Step 3:**

#### Unit Test Updates

**May need to update existing tests:**

1. **Check Transform Tests:**
   - Verify default sleep state is false
   - Test sleep methods work correctly

2. **Check Physics Tests:**
   - Tests expecting immediate physics response may fail if entities sleep
   - May need to explicitly wake entities in test setup

**Add New Tests (optional):**

**File:** `tests/SleepingEntityTest.cpp` (create new)

```cpp
#include "Entity.h"
#include "EntityManager.h"
#include "components/CTransform.h"
#include "components/CGravity.h"
#include "components/CCircleCollider.h"
#include "systems/S2DPhysics.h"
#include <gtest/gtest.h>

TEST(SleepingEntityTest, EntityStartsAwake)
{
    EntityManager::instance().reset();
    auto entity = EntityManager::instance().addEntity("test");
    auto* transform = entity->addComponent<CTransform>();

    EXPECT_FALSE(transform->isSleeping());
}

TEST(SleepingEntityTest, LowVelocityEventuallySleeps)
{
    EntityManager::instance().reset();
    S2DPhysics::instance().setWorldBounds(Vec2(500, 500), Vec2(1000, 1000));

    auto entity = EntityManager::instance().addEntity("ball");
    auto* transform = entity->addComponent<CTransform>(Vec2(100, 100), Vec2(1, 1), 0);
    entity->addComponent<CCircleCollider>(10.0f);

    // Set very low velocity
    transform->setVelocity(Vec2(1.0f, 1.0f));

    // Should not be sleeping initially
    EXPECT_FALSE(transform->isSleeping());

    // Run physics for 1 second at low velocity
    for (int i = 0; i < 60; ++i)
    {
        S2DPhysics::instance().update(1.0f / 60.0f);
    }

    // Should be sleeping now
    EXPECT_TRUE(transform->isSleeping());
}

TEST(SleepingEntityTest, CollisionWakesEntity)
{
    EntityManager::instance().reset();
    S2DPhysics::instance().setWorldBounds(Vec2(500, 500), Vec2(1000, 1000));

    auto entity1 = EntityManager::instance().addEntity("ball1");
    auto* transform1 = entity1->addComponent<CTransform>(Vec2(100, 100), Vec2(1, 1), 0);
    entity1->addComponent<CCircleCollider>(10.0f);

    auto entity2 = EntityManager::instance().addEntity("ball2");
    auto* transform2 = entity2->addComponent<CTransform>(Vec2(120, 100), Vec2(1, 1), 0);
    entity2->addComponent<CCircleCollider>(10.0f);

    // Put entity1 to sleep manually
    transform1->setSleeping(true);
    transform2->setVelocity(Vec2(-50.0f, 0.0f)); // Move entity2 toward entity1

    EXPECT_TRUE(transform1->isSleeping());

    // Run physics until collision
    for (int i = 0; i < 10; ++i)
    {
        S2DPhysics::instance().update(1.0f / 60.0f);
    }

    // Entity1 should be awake after collision
    EXPECT_FALSE(transform1->isSleeping());
}
```

#### Integration Testing

1. **Build the engine:**
   ```bash
   docker-compose exec dev ./build_tools/build.sh
   ```

2. **Run all tests:**
   ```bash
   docker-compose exec dev ./build/tests/GameEngine_Tests
   ```
   - All tests should pass
   - Check for new sleeping-related test output in debug builds

3. **Test Example_Game with Debug Output:**
   ```bash
   docker-compose exec dev cmake -DCMAKE_BUILD_TYPE=Debug -B Example_Game/build-debug
   docker-compose exec dev cmake --build Example_Game/build-debug
   docker-compose exec dev ./Example_Game/build-debug/BounceGame
   ```
   - Watch console for sleep/wake messages
   - Verify entities go to sleep after settling
   - Verify entities wake on collision

4. **Performance Test:**
   ```bash
   docker-compose exec dev ./Example_Game/build_example.sh
   ```
   - Start with 100 balls
   - Wait for balls to settle (5-10 seconds)
   - Measure frame rate:
     - Initial (all awake): Should be lower
     - After settling (most sleeping): Should be much higher
   - Expected: 2-3x FPS improvement when 80%+ entities are sleeping

#### Manual Testing Scenarios

**Scenario 1: Sleep Detection**
- Run example with 100 balls
- Watch debug output (if Debug build)
- Verify entities print "[SLEEPING]" after settling
- Count sleeping vs awake entities over time

**Scenario 2: Wake on Collision**
- Wait for all balls to settle and sleep
- Manually add a new fast-moving ball (modify example)
- Verify sleeping balls wake when hit

**Scenario 3: Threshold Tuning**
- If entities sleep too quickly or slowly:
  - Adjust `SLEEP_VELOCITY_THRESHOLD` in updateSleepState()
  - Adjust `SLEEP_TIME_THRESHOLD` for faster/slower sleeping
- Recompile and test with new values

**Scenario 4: Static Object Interaction**
- Verify static floor never shows as sleeping (it's always skipped)
- Verify balls sleep when resting on static floor
- Verify balls wake when new ball collides with pile

**Expected Results:**
- After 3-5 seconds, 70-90% of entities should be sleeping
- Frame rate should improve significantly once entities sleep
- Sleeping entities should wake instantly on collision
- No visible physics glitches or stuck entities

**Success Criteria:**
- ✅ All unit tests pass
- ✅ Entities sleep after velocity stabilizes
- ✅ Sleeping entities wake on collision
- ✅ No stuck or incorrectly sleeping entities
- ✅ Significant performance improvement (40-60%) in settled state
- ✅ No physics behavior changes (sleeping is pure optimization)

---

## Step 4: Deferred Optimizations

**Status:** Documented but not implemented in this issue

The following optimizations are noted for future work but add complexity beyond the scope of this issue:

### 4.1 Quadtree Caching and Incremental Updates

**Current Issue:**
- Entire quadtree cleared and rebuilt every frame
- All entities re-inserted even if they haven't moved

**Proposed Solution:**
- Maintain AABB bounds cache for entities
- Only rebuild quadtree nodes where entities moved
- Track which entities changed position each frame

**Complexity:** High (requires quadtree structure redesign)
**Expected Gain:** ~20-30% improvement

### 4.2 Separate Static and Dynamic Quadtrees

**Current Issue:**
- Static entities (floor, walls) re-inserted into quadtree every frame

**Proposed Solution:**
- Maintain two quadtrees: one for static, one for dynamic
- Static quadtree built once at level load
- Only rebuild dynamic quadtree each frame
- Query both trees during collision detection

**Complexity:** Medium
**Expected Gain:** ~10-15% improvement

### 4.3 Entity List Caching

**Current Issue:**
- `getEntitiesWithComponent<T>()` iterates all entities every call
- Called multiple times per frame for same component type

**Proposed Solution:**
- Cache component-to-entity mappings
- Update caches when entities added/removed
- Return cached lists instead of iterating

**Complexity:** Medium (requires EntityManager changes)
**Expected Gain:** ~5-10% improvement

### 4.4 Multithreading

**Current Issue:**
- All physics runs on single thread
- Cannot utilize multi-core CPUs

**Proposed Solution:**
- Parallelize broad-phase collision detection
- Solve independent collision islands in parallel
- Job system for entity updates

**Complexity:** Very High (threading, synchronization, race conditions)
**Expected Gain:** ~2-4x improvement on multi-core systems

### 4.5 GPU Physics (PhysX Integration)

**Current Issue:**
- CPU physics limits scalability to 100s of objects
- Cannot efficiently handle thousands of entities

**Proposed Solution:**
- Integrate Nvidia PhysX library
- Offload physics computation to GPU
- Handle complex scenarios (cloth, fluids, soft bodies)

**Complexity:** Very High (external dependency, API learning curve)
**Expected Gain:** 10-100x improvement for large-scale simulations

**Trade-offs:**
- Platform dependency (Windows/Linux)
- Licensing considerations
- Loss of control over low-level physics
- Integration complexity

---

## Performance Measurement

### Baseline Metrics (Before Optimization)

**Test Scenario:** Example_Game with 100 balls

Record these metrics BEFORE starting optimizations:

1. **Frame Rate (FPS):**
   - Initial frame rate (all balls falling): _____ FPS
   - Settled frame rate (balls at rest): _____ FPS

2. **Physics Update Time:**
   - Add timing code to `S2DPhysics::update()`:
     ```cpp
     auto start = std::chrono::high_resolution_clock::now();
     // ... physics code ...
     auto end = std::chrono::high_resolution_clock::now();
     auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
     ```
   - Average update time: _____ microseconds

3. **Console Output Volume:**
   - Lines of debug output per frame: ~_____ lines
   - Console scroll rate: Visual assessment

### Expected Results After Each Step

**After Step 1 (Debug Output):**
- ✅ Debug build: Same FPS (debug output still present)
- ✅ Release build: +5-10% FPS improvement
- ✅ Console: Silent in release build
- ✅ Physics timing: -5-10% (no I/O overhead)

**After Step 2 (Static Objects):**
- ✅ +10-20% FPS improvement from Step 1
- ✅ Physics timing: -10-20% (skipping 1% of entities)
- ✅ Behavior: Identical to baseline

**After Step 3 (Sleeping):**
- ✅ Initial FPS: Similar to Step 2 (all entities awake)
- ✅ Settled FPS: +40-60% from Step 2 (80% entities sleeping)
- ✅ Physics timing: -40-60% in settled state
- ✅ Behavior: Identical to baseline, entities wake on collision

**Combined Improvement:**
- Initial state: ~15-30% improvement (Steps 1+2)
- Settled state: ~70-150% improvement (2-2.5x faster) (Steps 1+2+3)

### Measurement Tool

**Add to Example_Game for testing:**

**File:** `Example_Game/src/main.cpp`

Add FPS counter to render():
```cpp
void render()
{
    m_window.clear(sf::Color(50, 50, 50));

    // ... existing rendering ...

    // Draw FPS counter
    if (m_fontLoaded)
    {
        std::ostringstream oss;
        oss << "FPS: " << (1.0f / m_lastDeltaTime);
        oss << " | SubSteps: " << (int)m_subStepCount;

        // Count sleeping entities
        int sleepingCount = 0;
        int totalCount = 0;
        auto entities = EntityManager::instance().getEntitiesByTag("ball");
        for (const auto& e : entities)
        {
            if (auto t = e->getComponent<CTransform>())
            {
                totalCount++;
                if (t->isSleeping())
                    sleepingCount++;
            }
        }
        oss << " | Sleeping: " << sleepingCount << "/" << totalCount;

        sf::Text text;
        text.setFont(m_font);
        text.setString(oss.str());
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);
        text.setPosition(10.0f, 10.0f);
        m_window.draw(text);
    }

    m_window.display();
}
```

Add member variable:
```cpp
private:
    float m_lastDeltaTime = 0.016f;
```

Update in run loop:
```cpp
void run()
{
    init();
    sf::Clock clock;
    while (m_running && m_window.isOpen())
    {
        m_lastDeltaTime = clock.restart().asSeconds();
        handleEvents();
        update(m_lastDeltaTime);
        render();
    }
    m_window.close();
}
```

---

## Rollback Plan

If any step causes issues:

### Rollback Step 1 (Debug Output)
```bash
git diff src/S2DPhysics.cpp > step1.patch
git checkout HEAD -- src/S2DPhysics.cpp CMakeLists.txt Example_Game/CMakeLists.txt
```

### Rollback Step 2 (Static Objects)
```bash
git checkout HEAD -- src/S2DPhysics.cpp
```

### Rollback Step 3 (Sleeping)
```bash
git checkout HEAD -- include/components/CTransform.h src/CTransform.cpp include/systems/S2DPhysics.h src/S2DPhysics.cpp
```

### Complete Rollback
```bash
git reset --hard HEAD
```

---

## Commit Strategy

**Commit after each step:**

**After Step 1:**
```bash
git add CMakeLists.txt Example_Game/CMakeLists.txt src/S2DPhysics.cpp
git commit -m "Add conditional compilation for debug output in physics system

- Add DEBUG macro to CMake for Debug builds only
- Wrap all std::cout in S2DPhysics.cpp with #ifdef DEBUG
- Release builds now have no console output overhead
- Debug builds retain full logging for development

Relates to #44"
```

**After Step 2:**
```bash
git add src/S2DPhysics.cpp
git commit -m "Skip physics updates for static objects

- Add static object checks in integratePositions()
- Add static object checks in handleGravity()
- Static entities (floors, walls) no longer processed unnecessarily
- Add TODO comment for future quadtree optimization

Performance: ~10-20% improvement by skipping static entities

Relates to #44"
```

**After Step 3:**
```bash
git add include/components/CTransform.h src/CTransform.cpp include/systems/S2DPhysics.h src/S2DPhysics.cpp
git commit -m "Implement entity sleeping system for physics optimization

- Add sleep state tracking to CTransform component
- Implement sleep detection based on velocity threshold
- Skip physics updates for sleeping entities
- Wake entities on collision or manual velocity change
- Add updateSleepState() to physics update loop

Performance: ~40-60% improvement once entities settle
Expected combined improvement: 2-5x faster physics in typical scenarios

Relates to #44"
```

---

## Final Testing Checklist

Before creating pull request:

### Code Quality
- [ ] All code follows project style guide
- [ ] No compiler warnings
- [ ] clang-format applied to all modified files
- [ ] cppcheck passes with no new warnings

### Functionality
- [ ] All 145+ unit tests pass
- [ ] Example_Game runs without crashes
- [ ] Physics behavior unchanged (no visual differences)
- [ ] Static objects remain stationary
- [ ] Sleeping entities wake on collision
- [ ] No entities get stuck in sleep state

### Performance
- [ ] Debug build has debug output
- [ ] Release build has no console output
- [ ] Measurable FPS improvement (record before/after)
- [ ] Sleeping entities reduce physics CPU time
- [ ] Settled state shows 70-90% sleeping entities

### Documentation
- [ ] This plan document updated with actual results
- [ ] Code comments explain sleep thresholds
- [ ] Commit messages are clear and reference #44

### Build Verification
- [ ] Docker build succeeds
- [ ] Example_Game builds successfully
- [ ] Tests build successfully
- [ ] No new build warnings

### Git Hygiene
- [ ] Three clear commits (one per step)
- [ ] No unrelated changes included
- [ ] Branch name follows convention
- [ ] Ready for pull request

---

## Pull Request Template

When creating PR, use this template:

```markdown
## Description
Implements physics performance optimizations through conditional debug output, static object skipping, and entity sleeping system.

Fixes #44

## Changes
1. **Conditional Debug Output**
   - Added DEBUG macro to CMake for Debug builds
   - Wrapped all physics debug output with `#ifdef DEBUG`
   - Release builds now have no console I/O overhead

2. **Static Object Optimization**
   - Skip static entities in `integratePositions()`
   - Skip static entities in `handleGravity()`
   - Prevents unnecessary computation for immovable objects

3. **Entity Sleeping System**
   - Extended `CTransform` with sleep state tracking
   - Implemented sleep detection based on velocity threshold
   - Entities sleep after 0.5s below 5.0 units/sec
   - Automatic wake on collision
   - Skip physics updates for sleeping entities

## Performance Results
**Test Configuration:** 100 balls in Example_Game

| Metric | Before | After Step 1 | After Step 2 | After Step 3 (Settled) |
|--------|--------|--------------|--------------|------------------------|
| FPS (Release) | ___ | ___ | ___ | ___ |
| Physics Time (μs) | ___ | ___ | ___ | ___ |
| Sleeping Entities | 0% | 0% | 0% | __% |

**Combined Improvement:** ~___% faster in settled state

## Testing
- [x] All 145+ unit tests pass
- [x] Example_Game runs correctly
- [x] Physics behavior unchanged
- [x] No visual glitches
- [x] Performance improvement verified

## Checklist
- [x] Code follows style guide
- [x] All tests pass
- [x] No compiler warnings
- [x] Documentation updated
- [x] Commits are atomic and well-described
```

---

## Appendix A: Sleep Threshold Tuning Guide

If default sleep thresholds don't work well for your game:

### Velocity Threshold (`SLEEP_VELOCITY_THRESHOLD`)

**Current:** 5.0 units/second

**Too Low (entities sleep while visibly moving):**
- Increase to 10.0 or 15.0
- Symptom: Balls appear to pause mid-roll

**Too High (entities never sleep):**
- Decrease to 2.0 or 3.0
- Symptom: FPS doesn't improve after settling

### Time Threshold (`SLEEP_TIME_THRESHOLD`)

**Current:** 0.5 seconds

**Too Short (sleep/wake flickering):**
- Increase to 1.0 or 2.0
- Symptom: Rapid sleep/wake cycles in debug output

**Too Long (delayed sleeping):**
- Decrease to 0.2 or 0.3
- Symptom: Performance improvement takes too long

### Wake Threshold (`WAKE_VELOCITY_THRESHOLD`)

**Current:** 10.0 units/second

**Should be higher than sleep threshold (hysteresis):**
- Rule: `WAKE_VELOCITY_THRESHOLD >= SLEEP_VELOCITY_THRESHOLD * 2`
- Prevents rapid oscillation between sleep/wake states

**Tuning Process:**
1. Run Example_Game with 100 balls
2. Enable debug output (Debug build)
3. Watch for sleep/wake messages
4. Adjust thresholds in `S2DPhysics::updateSleepState()`
5. Recompile and test
6. Iterate until behavior is smooth

---

## Appendix B: Common Issues and Solutions

### Issue: Entities Get Stuck Sleeping

**Symptom:** Sleeping entities don't wake on collision

**Causes:**
- Wake logic not called in `handleCollision()`
- Collision detection skips sleeping entities (wrong!)
- Wake threshold too high

**Solution:**
- Verify sleeping entities remain in quadtree
- Ensure `handleCollision()` calls `wake()` on both entities
- Check debug output for collision detection

### Issue: Entities Sleep Too Quickly

**Symptom:** Balls pause mid-bounce or mid-roll

**Causes:**
- `SLEEP_VELOCITY_THRESHOLD` too high
- `SLEEP_TIME_THRESHOLD` too short

**Solution:**
- Lower velocity threshold to 2.0-3.0
- Increase time threshold to 1.0-2.0

### Issue: Entities Never Sleep

**Symptom:** FPS doesn't improve after balls settle

**Causes:**
- Velocity never drops below threshold
- Collision micro-jittering keeping entities awake
- Sleep detection not called

**Solution:**
- Check that `updateSleepState()` is in physics loop
- Verify velocity actually drops (debug output)
- May need penetration depth tolerance in collision

### Issue: Performance Worse After Sleeping

**Symptom:** FPS drops after implementing sleeping

**Causes:**
- Sleep detection is too expensive
- Too many sleep/wake transitions
- Velocity calculation overhead

**Solution:**
- Profile `updateSleepState()` execution time
- Increase sleep time threshold (reduce transitions)
- Use `lengthSquared()` instead of `length()` where possible

---

## Appendix C: File Modification Summary

**Files Modified:**

1. `CMakeLists.txt` (root)
   - Add DEBUG macro for Debug builds

2. `Example_Game/CMakeLists.txt`
   - Add DEBUG macro for Debug builds

3. `include/components/CTransform.h`
   - Add sleep state members (m_isSleeping, m_sleepTimer, m_previousVelocity)
   - Add sleep state methods (isSleeping, setSleeping, wake, etc.)

4. `src/CTransform.cpp`
   - Initialize sleep state in constructor
   - Implement sleep methods
   - Modify setVelocity() to wake on manual velocity change

5. `include/systems/S2DPhysics.h`
   - Add updateSleepState() method declaration

6. `src/S2DPhysics.cpp`
   - Wrap all debug output with `#ifdef DEBUG`
   - Add static object checks in handleGravity()
   - Add static object checks in integratePositions()
   - Add sleep checks in handleGravity()
   - Add sleep checks in integratePositions()
   - Add wake logic in handleCollision()
   - Implement updateSleepState() function
   - Add updateSleepState() call to update() loop

7. `Example_Game/src/main.cpp` (optional, for FPS display)
   - Add FPS counter
   - Add sleeping entity counter
   - Add m_lastDeltaTime member

**Files NOT Modified:**
- All test files (unless sleeping tests are added)
- All other component files
- EntityManager
- CollisionDetector
- Quadtree
- All other header files

**Lines of Code Changed:** ~200-250 lines across all files

---

## Conclusion

This plan provides a comprehensive, step-by-step approach to optimizing physics performance in the 2D Game Engine. By implementing these three optimization strategies sequentially, we can achieve a 2-5x performance improvement in typical game scenarios while maintaining code quality and physics accuracy.

The plan is designed to be:
- **Incremental:** Test after each step
- **Measurable:** Clear performance metrics
- **Reversible:** Easy rollback if issues arise
- **Documented:** Detailed explanations for future maintainers

Upon completion, the engine will efficiently handle hundreds of physics objects with minimal CPU overhead, setting the foundation for more complex games and simulations.
