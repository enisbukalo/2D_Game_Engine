#include "systems/SInputManager.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <algorithm>

SInputManager* SInputManager::s_instance = nullptr;

SInputManager::SInputManager() = default;

SInputManager::~SInputManager() = default;

SInputManager& SInputManager::instance()
{
    if (!s_instance)
    {
        s_instance = new SInputManager();
    }
    return *s_instance;
}

void SInputManager::initialize(sf::RenderWindow* window, bool passToImGui)
{
    m_window      = window;
    m_passToImGui = passToImGui;
}

void SInputManager::shutdown()
{
    m_window = nullptr;
    m_keyDown.clear();
    m_keyPressed.clear();
    m_keyReleased.clear();
    m_keyRepeat.clear();
    m_mouseDown.clear();
    m_mousePressed.clear();
    m_mouseReleased.clear();
    m_mousePosition = {0, 0};
    m_actionBindings.clear();
    m_actionStates.clear();
    m_subscribers.clear();
    m_listenerPointers.clear();
    m_nextBindingId = 1;
}

static KeyCode keyCodeFromSFML(sf::Keyboard::Key k)
{
    using K = sf::Keyboard;
    switch (k)
    {
        case K::A:
            return KeyCode::A;
        case K::B:
            return KeyCode::B;
        case K::C:
            return KeyCode::C;
        case K::D:
            return KeyCode::D;
        case K::E:
            return KeyCode::E;
        case K::F:
            return KeyCode::F;
        case K::G:
            return KeyCode::G;
        case K::H:
            return KeyCode::H;
        case K::I:
            return KeyCode::I;
        case K::J:
            return KeyCode::J;
        case K::K:
            return KeyCode::K;
        case K::L:
            return KeyCode::L;
        case K::M:
            return KeyCode::M;
        case K::N:
            return KeyCode::N;
        case K::O:
            return KeyCode::O;
        case K::P:
            return KeyCode::P;
        case K::Q:
            return KeyCode::Q;
        case K::R:
            return KeyCode::R;
        case K::S:
            return KeyCode::S;
        case K::T:
            return KeyCode::T;
        case K::U:
            return KeyCode::U;
        case K::V:
            return KeyCode::V;
        case K::W:
            return KeyCode::W;
        case K::X:
            return KeyCode::X;
        case K::Y:
            return KeyCode::Y;
        case K::Z:
            return KeyCode::Z;
        case K::Num0:
            return KeyCode::Num0;
        case K::Num1:
            return KeyCode::Num1;
        case K::Num2:
            return KeyCode::Num2;
        case K::Num3:
            return KeyCode::Num3;
        case K::Num4:
            return KeyCode::Num4;
        case K::Num5:
            return KeyCode::Num5;
        case K::Num6:
            return KeyCode::Num6;
        case K::Num7:
            return KeyCode::Num7;
        case K::Num8:
            return KeyCode::Num8;
        case K::Num9:
            return KeyCode::Num9;
        case K::Escape:
            return KeyCode::Escape;
        case K::Space:
            return KeyCode::Space;
        case K::Enter:
            return KeyCode::Enter;
        case K::Backspace:
            return KeyCode::Backspace;
        case K::Tab:
            return KeyCode::Tab;
        case K::Left:
            return KeyCode::Left;
        case K::Right:
            return KeyCode::Right;
        case K::Up:
            return KeyCode::Up;
        case K::Down:
            return KeyCode::Down;
        default:
            return KeyCode::Unknown;
    }
}

static MouseButton mouseButtonFromSFML(sf::Mouse::Button mb)
{
    switch (mb)
    {
        case sf::Mouse::Left:
            return MouseButton::Left;
        case sf::Mouse::Right:
            return MouseButton::Right;
        case sf::Mouse::Middle:
            return MouseButton::Middle;
        case sf::Mouse::XButton1:
            return MouseButton::XButton1;
        case sf::Mouse::XButton2:
            return MouseButton::XButton2;
        default:
            return MouseButton::Unknown;
    }
}

BindingId SInputManager::bindAction(const std::string& actionName, const ActionBinding& binding)
{
    BindingId id = m_nextBindingId++;
    m_actionBindings[actionName].push_back({id, binding});
    m_actionStates.try_emplace(actionName, ActionState::None);
    return id;
}

void SInputManager::unbindAction(const std::string& actionName, BindingId id)
{
    auto it = m_actionBindings.find(actionName);
    if (it == m_actionBindings.end())
        return;
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(), [id](const auto& p) { return p.first == id; }), vec.end());
    if (vec.empty())
    {
        m_actionBindings.erase(it);
        m_actionStates.erase(actionName);
    }
}

void SInputManager::unbindAction(const std::string& actionName)
{
    m_actionBindings.erase(actionName);
    m_actionStates.erase(actionName);
}

void SInputManager::processEvent(const sf::Event& event)
{
    bool imguiCaptured = false;
    if (m_passToImGui)
    {
        ImGui::SFML::ProcessEvent(event);
        imguiCaptured = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
    }

    InputEvent inputEvent{};

    switch (event.type)
    {
        case sf::Event::KeyPressed:
        {
            KeyEvent ke{};
            ke.key          = keyCodeFromSFML(event.key.code);
            ke.alt          = event.key.alt;
            ke.ctrl         = event.key.control;
            ke.shift        = event.key.shift;
            ke.system       = event.key.system;
            ke.repeat       = false;  // SFML 2.6 doesn't provide repeat info
            inputEvent.type = InputEventType::KeyPressed;
            inputEvent.key  = ke;

            if (m_passToImGui && imguiCaptured)
                return;

            m_keyDown[ke.key]    = true;
            m_keyPressed[ke.key] = true;
            break;
        }
        case sf::Event::KeyReleased:
        {
            KeyEvent ke{};
            ke.key          = keyCodeFromSFML(event.key.code);
            ke.alt          = event.key.alt;
            ke.ctrl         = event.key.control;
            ke.shift        = event.key.shift;
            ke.system       = event.key.system;
            ke.repeat       = false;  // SFML 2.6 doesn't provide repeat info
            inputEvent.type = InputEventType::KeyReleased;
            inputEvent.key  = ke;

            if (m_passToImGui && imguiCaptured)
                return;

            m_keyDown[ke.key]     = false;
            m_keyReleased[ke.key] = true;
            m_keyRepeat.erase(ke.key);
            break;
        }
        case sf::Event::MouseButtonPressed:
        {
            MouseEvent me{};
            me.button        = mouseButtonFromSFML(event.mouseButton.button);
            me.position      = {(int)event.mouseButton.x, (int)event.mouseButton.y};
            inputEvent.type  = InputEventType::MouseButtonPressed;
            inputEvent.mouse = me;
            if (m_passToImGui && imguiCaptured)
                return;
            m_mouseDown[me.button]    = true;
            m_mousePressed[me.button] = true;
            m_mousePosition           = me.position;
            break;
        }
        case sf::Event::MouseButtonReleased:
        {
            MouseEvent me{};
            me.button        = mouseButtonFromSFML(event.mouseButton.button);
            me.position      = {(int)event.mouseButton.x, (int)event.mouseButton.y};
            inputEvent.type  = InputEventType::MouseButtonReleased;
            inputEvent.mouse = me;
            if (m_passToImGui && imguiCaptured)
                return;
            m_mouseDown[me.button]     = false;
            m_mouseReleased[me.button] = true;
            m_mousePosition            = me.position;
            break;
        }
        case sf::Event::MouseMoved:
        {
            MouseMoveEvent mm{};
            mm.position          = {event.mouseMove.x, event.mouseMove.y};
            inputEvent.type      = InputEventType::MouseMoved;
            inputEvent.mouseMove = mm;
            m_mousePosition      = mm.position;
            if (m_passToImGui && imguiCaptured)
                return;
            break;
        }
        case sf::Event::MouseWheelScrolled:
        {
            WheelEvent we{};
            we.delta         = event.mouseWheelScroll.delta;
            we.position      = {(int)event.mouseWheelScroll.x, (int)event.mouseWheelScroll.y};
            inputEvent.type  = InputEventType::MouseWheel;
            inputEvent.wheel = we;
            if (m_passToImGui && imguiCaptured)
                return;
            break;
        }
        case sf::Event::TextEntered:
        {
            TextEvent te{};
            te.unicode      = event.text.unicode;
            inputEvent.type = InputEventType::TextEntered;
            inputEvent.text = te;
            if (m_passToImGui && imguiCaptured)
                return;
            break;
        }
        case sf::Event::Closed:
        {
            inputEvent.type   = InputEventType::WindowClosed;
            inputEvent.window = WindowEvent{};
            if (m_passToImGui && imguiCaptured)
                return;
            break;
        }
        case sf::Event::Resized:
        {
            WindowEvent we{};
            we.width          = event.size.width;
            we.height         = event.size.height;
            inputEvent.type   = InputEventType::WindowResized;
            inputEvent.window = we;
            if (m_passToImGui && imguiCaptured)
                return;
            break;
        }
        default:
            return;
    }

    // dispatch function subscribers & pointer listeners
    for (const auto& kv : m_subscribers)
    {
        try
        {
            kv.second(inputEvent);
        }
        catch (...)
        {
        }
    }
    for (auto* l : m_listenerPointers)
    {
        if (!l)
            continue;
        switch (inputEvent.type)
        {
            case InputEventType::KeyPressed:
                l->onKeyPressed(inputEvent.key);
                break;
            case InputEventType::KeyReleased:
                l->onKeyReleased(inputEvent.key);
                break;
            case InputEventType::MouseButtonPressed:
                l->onMousePressed(inputEvent.mouse);
                break;
            case InputEventType::MouseButtonReleased:
                l->onMouseReleased(inputEvent.mouse);
                break;
            case InputEventType::MouseMoved:
                l->onMouseMoved(inputEvent.mouseMove);
                break;
            case InputEventType::TextEntered:
                l->onTextEntered(inputEvent.text);
                break;
            case InputEventType::WindowClosed:
            case InputEventType::WindowResized:
                l->onWindowEvent(inputEvent.window);
                break;
            default:
                break;
        }
    }
}

void SInputManager::update(float /*deltaTime*/)
{
    // Clear transient states (pressed/released) at start of update
    for (auto& kv : m_actionStates)
        if (kv.second == ActionState::Released)
            kv.second = ActionState::None;
    m_keyPressed.clear();
    m_keyReleased.clear();
    m_keyRepeat.clear();
    m_mousePressed.clear();
    m_mouseReleased.clear();
    if (!m_window)
        return;
    sf::Event event;
    while (m_window->pollEvent(event))
        processEvent(event);

    // Evaluate actions centrally
    for (const auto& actionKv : m_actionBindings)
    {
        const std::string& name     = actionKv.first;
        const auto&        bindings = actionKv.second;
        ActionState        newState = ActionState::None;

        for (const auto& pr : bindings)
        {
            const auto& binding     = pr.second;
            bool        anyDown     = false;
            bool        anyPressed  = false;
            bool        anyReleased = false;
            for (auto k : binding.keys)
            {
                if (m_keyDown.find(k) != m_keyDown.end() && m_keyDown[k])
                    anyDown = true;
                if (m_keyPressed.find(k) != m_keyPressed.end() && m_keyPressed[k])
                    anyPressed = true;
                if (m_keyReleased.find(k) != m_keyReleased.end() && m_keyReleased[k])
                    anyReleased = true;
                if (m_keyRepeat.find(k) != m_keyRepeat.end() && m_keyRepeat[k])
                    anyPressed = true;
            }
            for (auto mb : binding.mouseButtons)
            {
                if (m_mouseDown.find(mb) != m_mouseDown.end() && m_mouseDown[mb])
                    anyDown = true;
                if (m_mousePressed.find(mb) != m_mousePressed.end() && m_mousePressed[mb])
                    anyPressed = true;
                if (m_mouseReleased.find(mb) != m_mouseReleased.end() && m_mouseReleased[mb])
                    anyReleased = true;
            }
            switch (binding.trigger)
            {
                case ActionTrigger::Pressed:
                {
                    bool eff = anyPressed;
                    if (!binding.allowRepeat)
                    {
                        // if any key repeat triggered and repeat not allowed, ignore as pressed
                        bool repeatTriggered = false;
                        for (auto k : binding.keys)
                            if (m_keyRepeat.find(k) != m_keyRepeat.end() && m_keyRepeat[k])
                            {
                                repeatTriggered = true;
                                break;
                            }
                        if (repeatTriggered)
                            eff = false;
                    }
                    if (eff)
                        newState = ActionState::Pressed;
                    else if (anyDown)
                        newState = ActionState::Held;
                }
                break;
                case ActionTrigger::Held:
                    if (anyDown)
                        newState = anyPressed ? ActionState::Pressed : ActionState::Held;
                    break;
                case ActionTrigger::Released:
                    if (anyReleased)
                        newState = ActionState::Released;
                    break;
            }
            if (newState != ActionState::None)
                break;
        }

        ActionState previous = ActionState::None;
        auto        pit      = m_actionStates.find(name);
        if (pit != m_actionStates.end())
            previous = pit->second;
        bool wasDown = (previous == ActionState::Pressed || previous == ActionState::Held);
        if (!wasDown && newState == ActionState::Pressed)
            m_actionStates[name] = ActionState::Pressed;
        else if (wasDown && newState == ActionState::None)
            m_actionStates[name] = ActionState::Released;
        else
            m_actionStates[name] = newState;

        if (m_actionStates[name] != ActionState::None)
        {
            InputEvent ie{};
            ie.type              = InputEventType::Action;
            ie.action.actionName = name;
            ie.action.state      = m_actionStates[name];
            for (const auto& kv : m_subscribers)
            {
                try
                {
                    kv.second(ie);
                }
                catch (...)
                {
                }
            }
            for (auto* l : m_listenerPointers)
                if (l)
                    l->onAction(ie.action);
        }
    }
}

ListenerId SInputManager::subscribe(std::function<void(const InputEvent&)> cb)
{
    ListenerId id = m_nextListenerId++;
    m_subscribers.insert({id, cb});
    return id;
}

void SInputManager::unsubscribe(ListenerId id)
{
    m_subscribers.erase(id);
}

void SInputManager::addListener(IInputListener* listener)
{
    if (!listener)
        return;
    m_listenerPointers.push_back(listener);
}

void SInputManager::removeListener(IInputListener* listener)
{
    m_listenerPointers.erase(std::remove(m_listenerPointers.begin(), m_listenerPointers.end(), listener),
                             m_listenerPointers.end());
}

bool SInputManager::isKeyDown(KeyCode key) const
{
    auto it = m_keyDown.find(key);
    return (it != m_keyDown.end()) && it->second;
}

bool SInputManager::wasKeyPressed(KeyCode key) const
{
    auto it = m_keyPressed.find(key);
    return (it != m_keyPressed.end()) && it->second;
}

bool SInputManager::wasKeyReleased(KeyCode key) const
{
    auto it = m_keyReleased.find(key);
    return (it != m_keyReleased.end()) && it->second;
}

bool SInputManager::isMouseDown(MouseButton button) const
{
    auto it = m_mouseDown.find(button);
    return (it != m_mouseDown.end()) && it->second;
}

bool SInputManager::wasMousePressed(MouseButton button) const
{
    auto it = m_mousePressed.find(button);
    return (it != m_mousePressed.end()) && it->second;
}

bool SInputManager::wasMouseReleased(MouseButton button) const
{
    auto it = m_mouseReleased.find(button);
    return (it != m_mouseReleased.end()) && it->second;
}

sf::Vector2i SInputManager::getMousePositionWindow() const
{
    return m_mousePosition;
}

ActionState SInputManager::getActionState(const std::string& actionName) const
{
    auto it = m_actionStates.find(actionName);
    if (it == m_actionStates.end())
        return ActionState::None;
    return it->second;
}
