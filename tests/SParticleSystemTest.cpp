#include <gtest/gtest.h>
#include "SParticleSystem.h"
#include "CParticleEmitter.h"
#include "Color.h"
#include "Vec2.h"
#include <SFML/Graphics.hpp>
#include <cmath>

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
        SParticleSystem::instance().initialize(m_window, 100.0f);
    }

    void TearDown() override
    {
        // Shutdown system
        SParticleSystem::instance().shutdown();
        
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
    auto& ps = SParticleSystem::instance();
    EXPECT_TRUE(ps.isInitialized());
}

TEST_F(SParticleSystemTest, ReinitializeSucceeds)
{
    auto& ps = SParticleSystem::instance();
    
    // Re-initialize with different parameters
    ps.initialize(m_window, 200.0f);
    EXPECT_TRUE(ps.isInitialized());
}

//=============================================================================
// Update and Render Tests
//=============================================================================

TEST_F(SParticleSystemTest, UpdateDoesNotCrash)
{
    auto& ps = SParticleSystem::instance();
    
    // Should not crash even with no emitters
    EXPECT_NO_THROW(ps.update(0.016f));
}

TEST_F(SParticleSystemTest, RenderWithWindowDoesNotCrash)
{
    auto& ps = SParticleSystem::instance();
    
    // Should not crash
    EXPECT_NO_THROW(ps.render(m_window));
}

TEST_F(SParticleSystemTest, RenderWithoutWindowDoesNotCrash)
{
    auto& ps = SParticleSystem::instance();
    
    // Should not crash even with nullptr window
    EXPECT_NO_THROW(ps.render(nullptr));
}
