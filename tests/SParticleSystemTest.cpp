#include <gtest/gtest.h>
#include "SParticleSystem.h"
#include "Color.h"
#include "Vec2.h"
#include <SFML/Graphics.hpp>
#include <cmath>

/**
 * @brief Test fixture for SParticleSystem tests
 */
class SParticleSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize particle system with test parameters
        SParticleSystem::instance().initialize(100.0f, 600.0f);
    }

    void TearDown() override
    {
        // Clear all emitters and shutdown
        SParticleSystem::instance().clearEmitters();
        SParticleSystem::instance().shutdown();
    }

    /**
     * @brief Helper to create a basic emitter configuration
     */
    ParticleEmitterConfig createBasicConfig()
    {
        ParticleEmitterConfig config;
        config.position = Vec2(5.0f, 5.0f);
        config.direction = Vec2(0, 1);
        config.spreadAngle = 0.5f;
        config.minSpeed = 0.1f;
        config.maxSpeed = 0.5f;
        config.minLifetime = 0.5f;
        config.maxLifetime = 1.0f;
        config.minSize = 0.02f;
        config.maxSize = 0.05f;
        config.emissionRate = 10.0f;
        config.startColor = Color::White;
        config.endColor = Color::Blue;
        config.startAlpha = 1.0f;
        config.endAlpha = 0.0f;
        config.gravity = Vec2(0, 0.1f);
        config.fadeOut = true;
        config.shrink = true;
        config.active = true;
        config.maxParticles = 100;
        return config;
    }
};

//=============================================================================
// Initialization Tests
//=============================================================================

TEST_F(SParticleSystemTest, InitializationSucceeds)
{
    auto& ps = SParticleSystem::instance();
    EXPECT_EQ(ps.getPixelsPerMeter(), 100.0f);
    EXPECT_EQ(ps.getScreenHeight(), 600.0f);
}

TEST_F(SParticleSystemTest, ReinitializeUpdatesParameters)
{
    auto& ps = SParticleSystem::instance();
    ps.initialize(200.0f, 800.0f);
    EXPECT_EQ(ps.getPixelsPerMeter(), 200.0f);
    EXPECT_EQ(ps.getScreenHeight(), 800.0f);
}

TEST_F(SParticleSystemTest, SetPixelsPerMeter)
{
    auto& ps = SParticleSystem::instance();
    ps.setPixelsPerMeter(150.0f);
    EXPECT_EQ(ps.getPixelsPerMeter(), 150.0f);
}

TEST_F(SParticleSystemTest, SetScreenHeight)
{
    auto& ps = SParticleSystem::instance();
    ps.setScreenHeight(720.0f);
    EXPECT_EQ(ps.getScreenHeight(), 720.0f);
}

//=============================================================================
// Emitter Creation and Management Tests
//=============================================================================

TEST_F(SParticleSystemTest, CreateEmitterReturnsValidPointer)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    EXPECT_EQ(ps.getEmitterCount(), 1);
}

TEST_F(SParticleSystemTest, CreateMultipleEmitters)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    
    ps.createEmitter(config);
    ps.createEmitter(config);
    ps.createEmitter(config);
    
    EXPECT_EQ(ps.getEmitterCount(), 3);
}

TEST_F(SParticleSystemTest, RemoveEmitter)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    
    ParticleEmitter* emitter1 = ps.createEmitter(config);
    ParticleEmitter* emitter2 = ps.createEmitter(config);
    
    EXPECT_EQ(ps.getEmitterCount(), 2);
    
    ps.removeEmitter(emitter1);
    EXPECT_EQ(ps.getEmitterCount(), 1);
    
    ps.removeEmitter(emitter2);
    EXPECT_EQ(ps.getEmitterCount(), 0);
}

TEST_F(SParticleSystemTest, ClearEmitters)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    
    ps.createEmitter(config);
    ps.createEmitter(config);
    ps.createEmitter(config);
    
    EXPECT_EQ(ps.getEmitterCount(), 3);
    
    ps.clearEmitters();
    EXPECT_EQ(ps.getEmitterCount(), 0);
}

//=============================================================================
// Emitter Configuration Tests
//=============================================================================

TEST_F(SParticleSystemTest, EmitterPosition)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.position = Vec2(10.0f, 15.0f);
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    Vec2 pos = emitter->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 10.0f);
    EXPECT_FLOAT_EQ(pos.y, 15.0f);
}

TEST_F(SParticleSystemTest, SetEmitterPosition)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    emitter->setPosition(Vec2(20.0f, 25.0f));
    Vec2 pos = emitter->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 20.0f);
    EXPECT_FLOAT_EQ(pos.y, 25.0f);
}

TEST_F(SParticleSystemTest, SetEmitterDirection)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    // Set direction and verify it's normalized
    emitter->setDirection(Vec2(3.0f, 4.0f));  // Length = 5
    const auto& emitterConfig = emitter->getConfig();
    
    // Direction should be normalized
    float length = std::sqrt(emitterConfig.direction.x * emitterConfig.direction.x + 
                             emitterConfig.direction.y * emitterConfig.direction.y);
    EXPECT_NEAR(length, 1.0f, 0.001f);
}

TEST_F(SParticleSystemTest, EmitterActiveState)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.active = false;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    EXPECT_FALSE(emitter->isActive());
    
    emitter->setActive(true);
    EXPECT_TRUE(emitter->isActive());
    
    emitter->setActive(false);
    EXPECT_FALSE(emitter->isActive());
}

//=============================================================================
// Particle Emission Tests
//=============================================================================

TEST_F(SParticleSystemTest, ManualEmission)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;  // No automatic emission
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    EXPECT_EQ(emitter->getAliveCount(), 0);
    
    emitter->emit();
    EXPECT_EQ(emitter->getAliveCount(), 1);
    
    emitter->emit();
    emitter->emit();
    EXPECT_EQ(emitter->getAliveCount(), 3);
}

TEST_F(SParticleSystemTest, BurstEmission)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;  // No automatic emission
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    EXPECT_EQ(emitter->getAliveCount(), 0);
    
    emitter->emitBurst(50);
    EXPECT_EQ(emitter->getAliveCount(), 50);
}

TEST_F(SParticleSystemTest, ContinuousEmission)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 100.0f;  // 100 particles per second
    config.active = true;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    EXPECT_EQ(emitter->getAliveCount(), 0);
    
    // Update for 0.1 seconds, should emit ~10 particles
    emitter->update(0.1f);
    
    size_t particleCount = emitter->getAliveCount();
    EXPECT_GE(particleCount, 8);   // At least 8 particles
    EXPECT_LE(particleCount, 12);  // At most 12 particles (allowing some variance)
}

TEST_F(SParticleSystemTest, InactiveEmitterDoesNotEmit)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 100.0f;
    config.active = false;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    emitter->update(1.0f);  // Update for 1 second
    
    EXPECT_EQ(emitter->getAliveCount(), 0);  // Should not emit any particles
}

TEST_F(SParticleSystemTest, MaxParticlesLimit)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.maxParticles = 10;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    // Try to emit more than max
    emitter->emitBurst(20);
    
    EXPECT_LE(emitter->getAliveCount(), 10);
}

//=============================================================================
// Particle Update Tests
//=============================================================================

TEST_F(SParticleSystemTest, ParticleAging)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.minLifetime = 1.0f;
    config.maxLifetime = 1.0f;  // Fixed lifetime
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    emitter->emit();
    EXPECT_EQ(emitter->getAliveCount(), 1);
    
    // Update for 0.5 seconds (half lifetime)
    emitter->update(0.5f);
    EXPECT_EQ(emitter->getAliveCount(), 1);
    
    // Update for another 0.6 seconds (past lifetime)
    emitter->update(0.6f);
    EXPECT_EQ(emitter->getAliveCount(), 0);  // Should be dead
}

TEST_F(SParticleSystemTest, ParticlePhysicsUpdate)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.minSpeed = 1.0f;
    config.maxSpeed = 1.0f;
    config.direction = Vec2(1, 0);  // Move right
    config.spreadAngle = 0.0f;      // No spread
    config.gravity = Vec2(0, -1.0f); // Downward gravity
    config.minLifetime = 5.0f;  // Long lifetime so particle doesn't die
    config.maxLifetime = 5.0f;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    emitter->emit();
    
    // Get initial state immediately after emission
    const auto& particles = emitter->getParticles();
    ASSERT_EQ(particles.size(), 1);
    Vec2 initialPos = particles[0].position;
    
    // Update for 0.1 second (much less than lifetime)
    emitter->update(0.1f);
    
    // Particle should have moved
    EXPECT_GT(particles[0].position.x, initialPos.x);  // Moved right
    EXPECT_LT(particles[0].position.y, initialPos.y);  // Moved down (gravity)
}

TEST_F(SParticleSystemTest, ParticleRotation)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.minRotationSpeed = 1.0f;
    config.maxRotationSpeed = 1.0f;
    config.minLifetime = 5.0f;  // Long lifetime so particle doesn't die
    config.maxLifetime = 5.0f;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    emitter->emit();
    
    // Get initial state immediately after emission
    const auto& particles = emitter->getParticles();
    ASSERT_EQ(particles.size(), 1);
    float initialRotation = particles[0].rotation;
    
    // Update for 0.1 second (much less than lifetime)
    emitter->update(0.1f);
    
    // Rotation should have changed
    EXPECT_NE(particles[0].rotation, initialRotation);
}

//=============================================================================
// Particle Effects Tests
//=============================================================================

TEST_F(SParticleSystemTest, FadeOutEffect)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.minLifetime = 1.0f;
    config.maxLifetime = 1.0f;
    config.startAlpha = 1.0f;
    config.endAlpha = 0.0f;
    config.fadeOut = true;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    emitter->emit();
    const auto& particles = emitter->getParticles();
    ASSERT_EQ(particles.size(), 1);
    
    float initialAlpha = particles[0].alpha;
    EXPECT_NEAR(initialAlpha, 1.0f, 0.1f);
    
    // Update to halfway through lifetime
    emitter->update(0.5f);
    
    // Alpha should be around 0.5
    EXPECT_LT(particles[0].alpha, initialAlpha);
    EXPECT_GT(particles[0].alpha, 0.0f);
}

TEST_F(SParticleSystemTest, ShrinkEffect)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.minLifetime = 1.0f;
    config.maxLifetime = 1.0f;
    config.minSize = 0.1f;
    config.maxSize = 0.1f;
    config.shrink = true;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    emitter->emit();
    const auto& particles = emitter->getParticles();
    ASSERT_EQ(particles.size(), 1);
    
    float initialSize = particles[0].size;
    
    // Update to halfway through lifetime
    emitter->update(0.5f);
    
    // Size should have decreased
    EXPECT_LT(particles[0].size, initialSize);
}

//=============================================================================
// System-Level Tests
//=============================================================================

TEST_F(SParticleSystemTest, SystemUpdateUpdatesAllEmitters)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 10.0f;
    config.active = true;
    
    ps.createEmitter(config);
    ps.createEmitter(config);
    
    EXPECT_EQ(ps.getTotalParticleCount(), 0);
    
    // Update system
    ps.update(1.0f);
    
    // Both emitters should have emitted particles
    EXPECT_GT(ps.getTotalParticleCount(), 0);
}

TEST_F(SParticleSystemTest, GetTotalParticleCount)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    
    ParticleEmitter* emitter1 = ps.createEmitter(config);
    ParticleEmitter* emitter2 = ps.createEmitter(config);
    
    emitter1->emitBurst(10);
    emitter2->emitBurst(15);
    
    EXPECT_EQ(ps.getTotalParticleCount(), 25);
}

TEST_F(SParticleSystemTest, ClearParticles)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    emitter->emitBurst(50);
    
    EXPECT_EQ(emitter->getAliveCount(), 50);
    
    emitter->clear();
    EXPECT_EQ(emitter->getAliveCount(), 0);
}

//=============================================================================
// Rendering Tests
//=============================================================================

TEST_F(SParticleSystemTest, RenderWithoutWindowDoesNotCrash)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    emitter->emitBurst(10);
    
    // Should not crash even with nullptr window
    EXPECT_NO_THROW(ps.render(nullptr));
}

TEST_F(SParticleSystemTest, RenderWithWindowDoesNotCrash)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    emitter->emitBurst(10);
    
    // Create a minimal window for testing
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::None);
    
    // Should not crash
    EXPECT_NO_THROW(ps.render(&window));
    
    window.close();
}

//=============================================================================
// Edge Case Tests
//=============================================================================

TEST_F(SParticleSystemTest, ZeroEmissionRate)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.active = true;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    
    emitter->update(10.0f);  // Update for a long time
    
    EXPECT_EQ(emitter->getAliveCount(), 0);  // Should not emit anything
}

TEST_F(SParticleSystemTest, VeryHighEmissionRate)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 10000.0f;  // Very high rate
    config.maxParticles = 100;
    config.active = true;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    
    emitter->update(1.0f);
    
    // Should be capped at maxParticles
    EXPECT_LE(emitter->getAliveCount(), 100);
}

TEST_F(SParticleSystemTest, ZeroLifetimeParticles)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.minLifetime = 0.0f;
    config.maxLifetime = 0.0f;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    
    emitter->emit();
    
    // Particle should die immediately
    emitter->update(0.001f);
    EXPECT_EQ(emitter->getAliveCount(), 0);
}

TEST_F(SParticleSystemTest, NegativeGravity)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.gravity = Vec2(0, -10.0f);  // Strong upward gravity
    config.minSpeed = 0.0f;
    config.maxSpeed = 0.0f;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    
    emitter->emit();
    const auto& particles = emitter->getParticles();
    ASSERT_EQ(particles.size(), 1);
    
    float initialY = particles[0].position.y;
    
    emitter->update(1.0f);
    
    // Should move downward (negative Y direction due to negative gravity)
    EXPECT_LT(particles[0].position.y, initialY);
}

//=============================================================================
// Configuration Tests
//=============================================================================

TEST_F(SParticleSystemTest, DefaultConfiguration)
{
    ParticleEmitterConfig config;
    
    // Verify default values
    EXPECT_EQ(config.position.x, 0.0f);
    EXPECT_EQ(config.position.y, 0.0f);
    EXPECT_TRUE(config.fadeOut);
    EXPECT_TRUE(config.shrink);
    EXPECT_TRUE(config.active);
    EXPECT_GT(config.emissionRate, 0.0f);
}

TEST_F(SParticleSystemTest, ModifyConfigurationAfterCreation)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    ASSERT_NE(emitter, nullptr);
    
    // Modify configuration
    auto& emitterConfig = emitter->getConfig();
    emitterConfig.emissionRate = 50.0f;
    emitterConfig.startColor = Color::Red;
    
    // Verify changes
    EXPECT_FLOAT_EQ(emitter->getConfig().emissionRate, 50.0f);
    EXPECT_EQ(emitter->getConfig().startColor, Color::Red);
}

//=============================================================================
// Performance Tests
//=============================================================================

TEST_F(SParticleSystemTest, ManyParticlesPerformance)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    config.maxParticles = 1000;
    
    ParticleEmitter* emitter = ps.createEmitter(config);
    
    // Emit many particles
    emitter->emitBurst(1000);
    EXPECT_EQ(emitter->getAliveCount(), 1000);
    
    // Update should complete quickly
    auto start = std::chrono::high_resolution_clock::now();
    emitter->update(0.016f);  // ~60 FPS
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete in reasonable time (< 10ms)
    EXPECT_LT(duration.count(), 10);
}

TEST_F(SParticleSystemTest, MultipleEmittersPerformance)
{
    auto& ps = SParticleSystem::instance();
    auto config = createBasicConfig();
    config.emissionRate = 0.0f;
    
    // Create many emitters
    std::vector<ParticleEmitter*> emitters;
    for (int i = 0; i < 10; ++i)
    {
        ParticleEmitter* emitter = ps.createEmitter(config);
        emitter->emitBurst(50);
        emitters.push_back(emitter);
    }
    
    EXPECT_EQ(ps.getTotalParticleCount(), 500);
    
    // Update system should complete quickly
    auto start = std::chrono::high_resolution_clock::now();
    ps.update(0.016f);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete in reasonable time (< 20ms for 10 emitters with 50 particles each)
    EXPECT_LT(duration.count(), 20);
}
