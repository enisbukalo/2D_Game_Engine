#include <gtest/gtest.h>
#include "CInputController.h"
#include "SInputManager.h"
#include "Input/ActionBinding.h"
#include "Input/InputEvents.h"
#include "SSerialization.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

// Test fixture for CInputController tests
class CInputControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Reset and initialize SInputManager with no window and ImGui disabled
        SInputManager::instance().shutdown();
        SInputManager::instance().initialize(nullptr, false);
    }

    void TearDown() override
    {
        // Ensure any components are destroyed before shutting down manager
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
};

// Test: CInputController lifecycle - construction, init, destruction
TEST_F(CInputControllerTest, LifecycleBasic)
{
    {
        CInputController controller;
        controller.init();
        
        EXPECT_EQ(controller.getType(), "CInputController");
    }
    // Controller destructor should remove listener safely
}

// Test: Bind action and query state through controller
TEST_F(CInputControllerTest, BindActionAndQueryState)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::Space);
    binding.trigger = ActionTrigger::Pressed;

    controller.bindAction("Jump", binding);

    // Query through controller
    EXPECT_FALSE(controller.isActionDown("Jump"));
    EXPECT_FALSE(controller.wasActionPressed("Jump"));
    EXPECT_FALSE(controller.wasActionReleased("Jump"));
}

// Test: Unbind action removes binding
TEST_F(CInputControllerTest, UnbindActionRemovesBinding)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::E);
    binding.trigger = ActionTrigger::Pressed;

    controller.bindAction("Interact", binding);
    controller.unbindAction("Interact");

    // After unbinding, queries should return false
    EXPECT_FALSE(controller.isActionDown("Interact"));
}

// Test: Set action callback and verify invocation through direct onAction call
TEST_F(CInputControllerTest, SetActionCallbackAndInvoke)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::Space);
    binding.trigger = ActionTrigger::Pressed;

    controller.bindAction("Jump", binding);

    bool callbackInvoked = false;
    ActionState receivedState = ActionState::None;

    controller.setActionCallback("Jump", [&](ActionState state) {
        callbackInvoked = true;
        receivedState = state;
    });

    // Simulate an action event directly (since we're not polling a real window)
    ActionEvent actionEvent;
    actionEvent.actionName = "Jump";
    actionEvent.state = ActionState::Pressed;

    controller.onAction(actionEvent);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(receivedState, ActionState::Pressed);
}

// Test: Serialize and deserialize CInputController
TEST_F(CInputControllerTest, SerializeDeserializeRoundtrip)
{
    // Create controller with bindings
    CInputController controller1;
    controller1.init();

    ActionBinding binding1;
    binding1.keys.push_back(KeyCode::W);
    binding1.keys.push_back(KeyCode::Up);
    binding1.trigger = ActionTrigger::Pressed;
    binding1.allowRepeat = false;

    controller1.bindAction("MoveForward", binding1);

    ActionBinding binding2;
    binding2.keys.push_back(KeyCode::Space);
    binding2.mouseButtons.push_back(MouseButton::Left);
    binding2.trigger = ActionTrigger::Released;
    binding2.allowRepeat = true;

    controller1.bindAction("Jump", binding2);

    // Serialize
    Serialization::JsonBuilder builder;
    controller1.serialize(builder);
    std::string json1 = builder.toString();

    // Deserialize into new controller
   Serialization::SSerialization::JsonValue jsonValue(json1);
    CInputController controller2;
    controller2.init();
    controller2.deserialize(jsonValue);

    // Serialize again
    Serialization::JsonBuilder builder2;
    controller2.serialize(builder2);
    std::string json2 = builder2.toString();

    // Parse both JSONs and verify content matches (order-independent)
   Serialization::SSerialization::JsonValue parsed1(json1);
   Serialization::SSerialization::JsonValue parsed2(json2);

    // Verify both have cInputController
    ASSERT_FALSE(parsed1["cInputController"].isNull());
    ASSERT_FALSE(parsed2["cInputController"].isNull());

    // Get actions arrays
    const auto& actions1 = parsed1["cInputController"]["actions"].getArray();
    const auto& actions2 = parsed2["cInputController"]["actions"].getArray();

    // Should have same number of actions
    ASSERT_EQ(actions1.size(), actions2.size());
    EXPECT_EQ(actions1.size(), 2);

    // Build maps to compare actions by name (order-independent)
    std::map<std::string, const Serialization::SSerialization::JsonValue*> actionMap1;
    std::map<std::string, const Serialization::SSerialization::JsonValue*> actionMap2;

    for (const auto& action : actions1) {
        actionMap1[action["action"].getString()] = &action;
    }
    for (const auto& action : actions2) {
        actionMap2[action["action"].getString()] = &action;
    }

    // Verify both have the same action names
    EXPECT_TRUE(actionMap1.find("MoveForward") != actionMap1.end());
    EXPECT_TRUE(actionMap1.find("Jump") != actionMap1.end());
    EXPECT_TRUE(actionMap2.find("MoveForward") != actionMap2.end());
    EXPECT_TRUE(actionMap2.find("Jump") != actionMap2.end());

    // Verify MoveForward action details
    const auto* mf1 = actionMap1["MoveForward"];
    const auto* mf2 = actionMap2["MoveForward"];
    EXPECT_EQ((*mf1)["trigger"].getString(), (*mf2)["trigger"].getString());
    EXPECT_EQ((*mf1)["allowRepeat"].getBool(), (*mf2)["allowRepeat"].getBool());
    EXPECT_EQ((*mf1)["keys"].getArray().size(), (*mf2)["keys"].getArray().size());
    EXPECT_EQ((*mf1)["mouse"].getArray().size(), (*mf2)["mouse"].getArray().size());

    // Verify Jump action details
    const auto* jump1 = actionMap1["Jump"];
    const auto* jump2 = actionMap2["Jump"];
    EXPECT_EQ((*jump1)["trigger"].getString(), (*jump2)["trigger"].getString());
    EXPECT_EQ((*jump1)["allowRepeat"].getBool(), (*jump2)["allowRepeat"].getBool());
    EXPECT_EQ((*jump1)["keys"].getArray().size(), (*jump2)["keys"].getArray().size());
    EXPECT_EQ((*jump1)["mouse"].getArray().size(), (*jump2)["mouse"].getArray().size());
}

// Test: Deserialize empty or invalid JSON
TEST_F(CInputControllerTest, DeserializeEmptyJson)
{
    CInputController controller;
    controller.init();

   Serialization::SSerialization::JsonValue emptyJson("{}");
    
    // Should not crash
    controller.deserialize(emptyJson);
}

// Test: Multiple bindings for same action in controller
TEST_F(CInputControllerTest, MultipleBindingsForSameAction)
{
    CInputController controller;
    controller.init();

    ActionBinding binding1;
    binding1.keys.push_back(KeyCode::W);
    binding1.trigger = ActionTrigger::Pressed;

    ActionBinding binding2;
    binding2.keys.push_back(KeyCode::Up);
    binding2.trigger = ActionTrigger::Pressed;

    controller.bindAction("Move", binding1);
    controller.bindAction("Move", binding2);

    // Both bindings should exist
    // Verify by unbinding - this should remove both
    controller.unbindAction("Move");
    EXPECT_FALSE(controller.isActionDown("Move"));
}

// Test: Action callback is not invoked after unbinding
TEST_F(CInputControllerTest, CallbackNotInvokedAfterUnbind)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::F);
    binding.trigger = ActionTrigger::Pressed;

    controller.bindAction("Use", binding);

    int callbackCount = 0;
    controller.setActionCallback("Use", [&](ActionState) {
        callbackCount++;
    });

    // Invoke callback
    ActionEvent actionEvent;
    actionEvent.actionName = "Use";
    actionEvent.state = ActionState::Pressed;
    controller.onAction(actionEvent);
    EXPECT_EQ(callbackCount, 1);

    // Unbind action
    controller.unbindAction("Use");

    // Try to invoke again
    controller.onAction(actionEvent);
    EXPECT_EQ(callbackCount, 1); // Should not increment
}

// Test: Set action callback to nullptr removes callback
TEST_F(CInputControllerTest, SetCallbackToNullptrRemovesCallback)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::G);
    binding.trigger = ActionTrigger::Pressed;

    controller.bindAction("Grenade", binding);

    int callbackCount = 0;
    controller.setActionCallback("Grenade", [&](ActionState) {
        callbackCount++;
    });

    ActionEvent actionEvent;
    actionEvent.actionName = "Grenade";
    actionEvent.state = ActionState::Pressed;

    controller.onAction(actionEvent);
    EXPECT_EQ(callbackCount, 1);

    // Remove callback
    controller.setActionCallback("Grenade", nullptr);

    // Try to invoke again
    controller.onAction(actionEvent);
    EXPECT_EQ(callbackCount, 1); // Should not increment
}

// Test: isActionDown returns correct value for Pressed state
TEST_F(CInputControllerTest, IsActionDownForPressedState)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::A);
    binding.trigger = ActionTrigger::Pressed;

    controller.bindAction("StrafeLeft", binding);

    // Simulate the action being pressed at the manager level
    // (In real code, this would be set by the input manager's evaluation)
    // For this test, we'll bind and check that the manager's getActionState is queried
    
    // We can't easily set the manager's action state without simulating the full event flow,
    // so this test primarily verifies the query methods exist and return values
    EXPECT_FALSE(controller.isActionDown("StrafeLeft"));
}

// Test: wasActionPressed query
TEST_F(CInputControllerTest, WasActionPressedQuery)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::R);
    binding.trigger = ActionTrigger::Pressed;

    controller.bindAction("Reload", binding);

    EXPECT_FALSE(controller.wasActionPressed("Reload"));
}

// Test: wasActionReleased query
TEST_F(CInputControllerTest, WasActionReleasedQuery)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::C);
    binding.trigger = ActionTrigger::Released;

    controller.bindAction("Crouch", binding);

    EXPECT_FALSE(controller.wasActionReleased("Crouch"));
}

// Test: Destructor properly cleans up bindings and listeners
TEST_F(CInputControllerTest, DestructorCleansUpProperly)
{
    {
        CInputController controller;
        controller.init();

        ActionBinding binding;
        binding.keys.push_back(KeyCode::Tab);
        binding.trigger = ActionTrigger::Pressed;

        controller.bindAction("Inventory", binding);
        
        // Controller goes out of scope here and should clean up
    }
    
    // If we reach here without crash, cleanup was successful
    SUCCEED();
}

// Test: Serialize with no bindings produces valid JSON
TEST_F(CInputControllerTest, SerializeWithNoBindings)
{
    CInputController controller;
    controller.init();

    Serialization::JsonBuilder builder;
    controller.serialize(builder);
    std::string json = builder.toString();

    // Should produce valid JSON even with no bindings
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("cInputController"), std::string::npos);
}

// Test: Binding with multiple keys serializes correctly
TEST_F(CInputControllerTest, SerializeMultipleKeys)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.keys.push_back(KeyCode::A);
    binding.keys.push_back(KeyCode::B);
    binding.keys.push_back(KeyCode::C);
    binding.trigger = ActionTrigger::Held;
    binding.allowRepeat = true;

    controller.bindAction("MultiKey", binding);

    Serialization::JsonBuilder builder;
    controller.serialize(builder);
    std::string json = builder.toString();

    // Verify the JSON contains the action name
    EXPECT_NE(json.find("MultiKey"), std::string::npos);
}

// Test: Binding with mouse buttons serializes correctly
TEST_F(CInputControllerTest, SerializeMouseButtons)
{
    CInputController controller;
    controller.init();

    ActionBinding binding;
    binding.mouseButtons.push_back(MouseButton::Left);
    binding.mouseButtons.push_back(MouseButton::Right);
    binding.trigger = ActionTrigger::Pressed;

    controller.bindAction("Shoot", binding);

    Serialization::JsonBuilder builder;
    controller.serialize(builder);
    std::string json = builder.toString();

    EXPECT_NE(json.find("Shoot"), std::string::npos);
}

// Test: Different trigger types serialize correctly
TEST_F(CInputControllerTest, SerializeDifferentTriggers)
{
    CInputController controller;
    controller.init();

    ActionBinding pressedBinding;
    pressedBinding.keys.push_back(KeyCode::Num1);
    pressedBinding.trigger = ActionTrigger::Pressed;
    controller.bindAction("Weapon1", pressedBinding);

    ActionBinding heldBinding;
    heldBinding.keys.push_back(KeyCode::Num2);
    heldBinding.trigger = ActionTrigger::Held;
    controller.bindAction("Weapon2", heldBinding);

    ActionBinding releasedBinding;
    releasedBinding.keys.push_back(KeyCode::Num3);
    releasedBinding.trigger = ActionTrigger::Released;
    controller.bindAction("Weapon3", releasedBinding);

    Serialization::JsonBuilder builder;
    controller.serialize(builder);
    std::string json = builder.toString();

    // All actions should be present
    EXPECT_NE(json.find("Weapon1"), std::string::npos);
    EXPECT_NE(json.find("Weapon2"), std::string::npos);
    EXPECT_NE(json.find("Weapon3"), std::string::npos);
}
