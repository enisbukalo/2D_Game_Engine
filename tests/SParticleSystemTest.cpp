#include <gtest/gtest.h>
#include "SParticle.h"
#include "CParticleEmitter.h"
#include "CTransform.h"
#include "Color.h"
#include "Entity.h"
#include "SEntity.h"
#include "Vec2.h"
#include <SFML/Graphics.hpp>
#include <cmath>

using namespace Components;
using namespace Entity;
using namespace Systems;

/**
 * @brief Test fixture for SParticleSystem tests
 *
 * Note: The current implementation uses an ECS-based approach with CParticleEmitter components.
 * These tests verify basic initialization and system-level functionality.
 */
class SParticleSystemTest : public ::testing::Test
{
protected:
    sf::RenderWindow* m_window;

    void SetUp() override
    {
        // Create minimal window for testing
        m_window = new sf::RenderWindow(sf::VideoMode(800, 600), "Test", sf::Style::None);

        // Initialize particle system with window and test parameters
        SParticle::instance().initialize(m_window, 100.0f);
    }

    void TearDown() override
    {
        // Clear all entities
        SEntity::instance().clear();
        SEntity::instance().update(0.0f);

        // Shutdown system
        SParticle::instance().shutdown();

        if (m_window)
        {
            m_window->close();
            delete m_window;
            m_window = nullptr;
        }
    }
};

//=============================================================================
// Initialization Tests
//=============================================================================

TEST_F(SParticleSystemTest, InitializationSucceeds)
{
    auto& ps = SParticle::instance();
    EXPECT_TRUE(ps.isInitialized());
}

TEST_F(SParticleSystemTest, ReinitializeSucceeds)
{
    auto& ps = SParticle::instance();

    // Re-initialize with different parameters
    ps.initialize(m_window, 200.0f);
    EXPECT_TRUE(ps.isInitialized());
}

//=============================================================================
// Update and Render Tests
//=============================================================================

TEST_F(SParticleSystemTest, UpdateDoesNotCrash)
{
    auto& ps = SParticle::instance();

    // Should not crash even with no emitters
    EXPECT_NO_THROW(ps.update(0.016f));
}

TEST_F(SParticleSystemTest, RenderEmitterWithWindowDoesNotCrash)
{
    auto& ps = SParticle::instance();

    // Create an entity with emitter for testing
    auto entity = SEntity::instance().addEntity("test_emitter");
    entity->addComponent<CTransform>(Vec2(0, 0), Vec2(1.0f, 1.0f), 0.0f);
    entity->addComponent<CParticleEmitter>();
    SEntity::instance().update(0.0f);

    // Should not crash
    EXPECT_NO_THROW(ps.renderEmitter(entity.get(), m_window));
}

TEST_F(SParticleSystemTest, RenderEmitterWithNullWindowDoesNotCrash)
{
    auto& ps = SParticle::instance();

    // Create an entity with emitter for testing
    auto entity = SEntity::instance().addEntity("test_emitter");
    entity->addComponent<CTransform>(Vec2(0, 0), Vec2(1.0f, 1.0f), 0.0f);
    entity->addComponent<CParticleEmitter>();
    SEntity::instance().update(0.0f);

    // Should not crash even with nullptr window
    EXPECT_NO_THROW(ps.renderEmitter(entity.get(), nullptr));
}

TEST_F(SParticleSystemTest, RenderEmitterWithNullEntityDoesNotCrash)
{
    auto& ps = SParticle::instance();

    // Should not crash with nullptr entity
    EXPECT_NO_THROW(ps.renderEmitter(nullptr, m_window));
}

TEST_F(SParticleSystemTest, RenderEmitterWithEntityWithoutEmitterDoesNotCrash)
{
    auto& ps = SParticle::instance();

    // Create an entity without emitter
    auto entity = SEntity::instance().addEntity("no_emitter");
    entity->addComponent<CTransform>(Vec2(0, 0), Vec2(1.0f, 1.0f), 0.0f);
    SEntity::instance().update(0.0f);

    // Should not crash
    EXPECT_NO_THROW(ps.renderEmitter(entity.get(), m_window));
}
