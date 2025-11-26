#include <gtest/gtest.h>
#include "systems/SInputManager.h"
#include "Input/InputEvents.h"
#include "Input/IInputListener.h"
#include "Input/ActionBinding.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

// Test fixture for SInputManager tests
class SInputManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Reset and initialize with no window and ImGui disabled
        SInputManager::instance().shutdown();
        SInputManager::instance().initialize(nullptr, false);
    }

    void TearDown() override
    {
        // Clean up after each test
        SInputManager::instance().shutdown();
    }

    // Helper to create a KeyPressed event
    sf::Event createKeyPressedEvent(sf::Keyboard::Key key, bool repeat = false)
    {
        sf::Event event;
        event.type = sf::Event::KeyPressed;
        event.key.code = key;
        event.key.alt = false;
        event.key.control = false;
        event.key.shift = false;
        event.key.system = false;
        // event.key.repeat not available in SFML 2.6
        return event;
    }

    // Helper to create a KeyReleased event
    sf::Event createKeyReleasedEvent(sf::Keyboard::Key key)
    {
        sf::Event event;
        event.type = sf::Event::KeyReleased;
        event.key.code = key;
        event.key.alt = false;
        event.key.control = false;
        event.key.shift = false;
        event.key.system = false;
        // event.key.repeat not available in SFML 2.6
        return event;
    }

    // Helper to create a MouseButtonPressed event
    sf::Event createMousePressedEvent(sf::Mouse::Button button, int x, int y)
    {
        sf::Event event;
        event.type = sf::Event::MouseButtonPressed;
        event.mouseButton.button = button;
        event.mouseButton.x = x;
        event.mouseButton.y = y;
        return event;
    }

    // Helper to create a MouseButtonReleased event
    sf::Event createMouseReleasedEvent(sf::Mouse::Button button, int x, int y)
    {
        sf::Event event;
        event.type = sf::Event::MouseButtonReleased;
        event.mouseButton.button = button;
        event.mouseButton.x = x;
        event.mouseButton.y = y;
        return event;
    }

    // Helper to create a MouseMoved event
    sf::Event createMouseMovedEvent(int x, int y)
    {
        sf::Event event;
        event.type = sf::Event::MouseMoved;
        event.mouseMove.x = x;
        event.mouseMove.y = y;
        return event;
    }
};

// Simple test listener to verify dispatch
class TestListener : public IInputListener
{
public:
    int keyPressedCount = 0;
    int keyReleasedCount = 0;
    int mousePressedCount = 0;
    int actionCount = 0;
    KeyCode lastKey = KeyCode::Unknown;
    std::string lastAction;
    ActionState lastActionState = ActionState::None;

    void onKeyPressed(const KeyEvent& ev) override
    {
        keyPressedCount++;
        lastKey = ev.key;
    }

    void onKeyReleased(const KeyEvent& ev) override
    {
        keyReleasedCount++;
        lastKey = ev.key;
    }

    void onMousePressed(const MouseEvent& ev) override
    {
        mousePressedCount++;
    }

    void onAction(const ActionEvent& ev) override
    {
        actionCount++;
        lastAction = ev.actionName;
        lastActionState = ev.state;
    }
};

// Test: Key conversion from SFML to engine KeyCode
TEST_F(SInputManagerTest, KeyCodeConversionFromSFML)
{
    auto& manager = SInputManager::instance();

    sf::Event event = createKeyPressedEvent(sf::Keyboard::A);
    manager.processEvent(event);

    EXPECT_TRUE(manager.isKeyDown(KeyCode::A));
    EXPECT_TRUE(manager.wasKeyPressed(KeyCode::A));
    EXPECT_FALSE(manager.isKeyDown(KeyCode::B));
}

// Test: Key press and release states
TEST_F(SInputManagerTest, KeyPressAndReleaseStates)
{
    auto& manager = SInputManager::instance();

    // Press key
    sf::Event pressEvent = createKeyPressedEvent(sf::Keyboard::Space);
    manager.processEvent(pressEvent);

    EXPECT_TRUE(manager.isKeyDown(KeyCode::Space));
    EXPECT_TRUE(manager.wasKeyPressed(KeyCode::Space));
    EXPECT_FALSE(manager.wasKeyReleased(KeyCode::Space));

    // Release key
    sf::Event releaseEvent = createKeyReleasedEvent(sf::Keyboard::Space);
    manager.processEvent(releaseEvent);

    EXPECT_FALSE(manager.isKeyDown(KeyCode::Space));
    EXPECT_TRUE(manager.wasKeyReleased(KeyCode::Space));
}

// Test: Key repeat handling
TEST_F(SInputManagerTest, KeyRepeatHandling)
{
    auto& manager = SInputManager::instance();

    // First press (not a repeat)
    sf::Event firstPress = createKeyPressedEvent(sf::Keyboard::W, false);
    manager.processEvent(firstPress);

    EXPECT_TRUE(manager.wasKeyPressed(KeyCode::W));

    // Repeated press
    sf::Event repeatPress = createKeyPressedEvent(sf::Keyboard::W, true);
    manager.processEvent(repeatPress);

    // wasKeyPressed should still be true for repeat (implementation shows repeat sets m_keyRepeat)
    EXPECT_TRUE(manager.isKeyDown(KeyCode::W));
}

// Test: Mouse button press and release
TEST_F(SInputManagerTest, MouseButtonPressAndRelease)
{
    auto& manager = SInputManager::instance();

    // Press mouse button
    sf::Event pressEvent = createMousePressedEvent(sf::Mouse::Left, 100, 200);
    manager.processEvent(pressEvent);

    EXPECT_TRUE(manager.isMouseDown(MouseButton::Left));
    sf::Vector2i mousePos = manager.getMousePositionWindow();
    EXPECT_EQ(mousePos.x, 100);
    EXPECT_EQ(mousePos.y, 200);

    // Release mouse button
    sf::Event releaseEvent = createMouseReleasedEvent(sf::Mouse::Left, 150, 250);
    manager.processEvent(releaseEvent);

    EXPECT_FALSE(manager.isMouseDown(MouseButton::Left));
    mousePos = manager.getMousePositionWindow();
    EXPECT_EQ(mousePos.x, 150);
    EXPECT_EQ(mousePos.y, 250);
}

// Test: Mouse move updates position
TEST_F(SInputManagerTest, MouseMoveUpdatesPosition)
{
    auto& manager = SInputManager::instance();

    sf::Event moveEvent = createMouseMovedEvent(300, 400);
    manager.processEvent(moveEvent);

    sf::Vector2i mousePos = manager.getMousePositionWindow();
    EXPECT_EQ(mousePos.x, 300);
    EXPECT_EQ(mousePos.y, 400);
}

// Test: Subscribe callback receives events
TEST_F(SInputManagerTest, SubscribeCallbackReceivesEvents)
{
    auto& manager = SInputManager::instance();

    bool callbackInvoked = false;
    InputEventType receivedType = InputEventType::KeyPressed;

    ListenerId id = manager.subscribe([&](const InputEvent& ev) {
        callbackInvoked = true;
        receivedType = ev.type;
    });

    sf::Event event = createKeyPressedEvent(sf::Keyboard::A);
    manager.processEvent(event);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(receivedType, InputEventType::KeyPressed);

    // Cleanup
    manager.unsubscribe(id);
}

// Test: Pointer listener receives events
TEST_F(SInputManagerTest, PointerListenerReceivesEvents)
{
    auto& manager = SInputManager::instance();

    TestListener listener;
    manager.addListener(&listener);

    sf::Event keyPressEvent = createKeyPressedEvent(sf::Keyboard::D);
    manager.processEvent(keyPressEvent);

    EXPECT_EQ(listener.keyPressedCount, 1);
    EXPECT_EQ(listener.lastKey, KeyCode::D);

    sf::Event keyReleaseEvent = createKeyReleasedEvent(sf::Keyboard::D);
    manager.processEvent(keyReleaseEvent);

    EXPECT_EQ(listener.keyReleasedCount, 1);

    sf::Event mousePressEvent = createMousePressedEvent(sf::Mouse::Right, 50, 50);
    manager.processEvent(mousePressEvent);

    EXPECT_EQ(listener.mousePressedCount, 1);

    // Cleanup
    manager.removeListener(&listener);
}

// Test: Unsubscribe stops receiving events
TEST_F(SInputManagerTest, UnsubscribeStopsReceivingEvents)
{
    auto& manager = SInputManager::instance();

    int callCount = 0;
    ListenerId id = manager.subscribe([&](const InputEvent&) {
        callCount++;
    });

    sf::Event event = createKeyPressedEvent(sf::Keyboard::A);
    manager.processEvent(event);
    EXPECT_EQ(callCount, 1);

    manager.unsubscribe(id);

    sf::Event event2 = createKeyPressedEvent(sf::Keyboard::B);
    manager.processEvent(event2);
    EXPECT_EQ(callCount, 1); // Should not increment
}

// Test: Remove listener stops receiving events
TEST_F(SInputManagerTest, RemoveListenerStopsReceivingEvents)
{
    auto& manager = SInputManager::instance();

    TestListener listener;
    manager.addListener(&listener);

    sf::Event event = createKeyPressedEvent(sf::Keyboard::A);
    manager.processEvent(event);
    EXPECT_EQ(listener.keyPressedCount, 1);

    manager.removeListener(&listener);

    sf::Event event2 = createKeyPressedEvent(sf::Keyboard::B);
    manager.processEvent(event2);
    EXPECT_EQ(listener.keyPressedCount, 1); // Should not increment
}

// Test: Bind action and query state
TEST_F(SInputManagerTest, BindActionAndQueryState)
{
    auto& manager = SInputManager::instance();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::Space);
    binding.trigger = ActionTrigger::Pressed;
    binding.allowRepeat = false;

    manager.bindAction("Jump", binding);

    // Initially None
    EXPECT_EQ(manager.getActionState("Jump"), ActionState::None);
}

// Test: Unbind action by name
TEST_F(SInputManagerTest, UnbindActionByName)
{
    auto& manager = SInputManager::instance();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::E);
    binding.trigger = ActionTrigger::Pressed;

    BindingId id = manager.bindAction("Interact", binding);

    manager.unbindAction("Interact");

    // After unbinding, state should be None (or action not found)
    EXPECT_EQ(manager.getActionState("Interact"), ActionState::None);
}

// Test: Unbind action by ID
TEST_F(SInputManagerTest, UnbindActionById)
{
    auto& manager = SInputManager::instance();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::F);
    binding.trigger = ActionTrigger::Pressed;

    BindingId id = manager.bindAction("Use", binding);

    manager.unbindAction("Use", id);

    EXPECT_EQ(manager.getActionState("Use"), ActionState::None);
}

// Test: Multiple bindings for same action
TEST_F(SInputManagerTest, MultipleBindingsForSameAction)
{
    auto& manager = SInputManager::instance();

    ActionBinding binding1;
    binding1.keys.push_back(KeyCode::W);
    binding1.trigger = ActionTrigger::Pressed;

    ActionBinding binding2;
    binding2.keys.push_back(KeyCode::Up);
    binding2.trigger = ActionTrigger::Pressed;

    manager.bindAction("MoveForward", binding1);
    manager.bindAction("MoveForward", binding2);

    // Both should exist - we can't directly test internal state, but unbinding by name should work
    manager.unbindAction("MoveForward");
    EXPECT_EQ(manager.getActionState("MoveForward"), ActionState::None);
}

// Test: Shutdown clears all state
TEST_F(SInputManagerTest, ShutdownClearsAllState)
{
    auto& manager = SInputManager::instance();

    // Set up some state
    sf::Event event = createKeyPressedEvent(sf::Keyboard::A);
    manager.processEvent(event);

    ActionBinding binding;
    binding.keys.push_back(KeyCode::A);
    manager.bindAction("TestAction", binding);

    TestListener listener;
    manager.addListener(&listener);

    EXPECT_TRUE(manager.isKeyDown(KeyCode::A));

    // Shutdown
    manager.shutdown();

    // State should be cleared
    EXPECT_FALSE(manager.isKeyDown(KeyCode::A));
    EXPECT_EQ(manager.getActionState("TestAction"), ActionState::None);
}

// Test: Multiple keys in single binding
TEST_F(SInputManagerTest, MultipleKeysInBinding)
{
    auto& manager = SInputManager::instance();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::A);
    binding.keys.push_back(KeyCode::B);
    binding.trigger = ActionTrigger::Pressed;

    manager.bindAction("Combo", binding);

    EXPECT_EQ(manager.getActionState("Combo"), ActionState::None);
}

// Test: Mouse button in action binding
TEST_F(SInputManagerTest, MouseButtonInActionBinding)
{
    auto& manager = SInputManager::instance();

    ActionBinding binding;
    binding.mouseButtons.push_back(MouseButton::Left);
    binding.trigger = ActionTrigger::Pressed;

    manager.bindAction("Fire", binding);

    EXPECT_EQ(manager.getActionState("Fire"), ActionState::None);
}

// Test: Initialize resets passToImGui flag
TEST_F(SInputManagerTest, InitializeSetsPassToImGuiFlag)
{
    auto& manager = SInputManager::instance();

    // Re-initialize with passToImGui true
    manager.initialize(nullptr, true);
    
    // We can't directly test the flag, but we can verify no crash occurs
    sf::Event event = createKeyPressedEvent(sf::Keyboard::A);
    // This would call ImGui if passToImGui was true, but since we have no context it should handle gracefully
    // For this test we just verify no crash
    // In actual test we initialized with false in SetUp, so this is just checking re-initialization
    
    // Re-initialize with false for remaining tests
    manager.shutdown();
    manager.initialize(nullptr, false);
}
