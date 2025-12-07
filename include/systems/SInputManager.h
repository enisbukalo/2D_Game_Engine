#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Input/ActionBinding.h"
#include "Input/IInputListener.h"
#include "Input/InputEvents.h"
#include "Input/MouseButton.h"
#include "System.h"

using ListenerId = size_t;
using BindingId  = size_t;

class SInputManager : public System
{
private:
    SInputManager();

    sf::RenderWindow* m_window      = nullptr;
    bool              m_passToImGui = true;

    // Key state maps: current down state and previous frame
    std::unordered_map<KeyCode, bool>     m_keyDown;
    std::unordered_map<KeyCode, bool>     m_keyPressed;
    std::unordered_map<KeyCode, bool>     m_keyReleased;
    std::unordered_map<KeyCode, bool>     m_keyRepeat;
    std::unordered_map<MouseButton, bool> m_mouseDown;
    std::unordered_map<MouseButton, bool> m_mousePressed;
    std::unordered_map<MouseButton, bool> m_mouseReleased;
    sf::Vector2i                          m_mousePosition;

    // Action bindings (per-action -> list of pairs(bindingId, binding))
    std::unordered_map<std::string, std::vector<std::pair<BindingId, ActionBinding>>> m_actionBindings;
    std::unordered_map<std::string, ActionState>                                      m_actionStates;
    BindingId                                                                         m_nextBindingId = 1;

    // Listener collections
    std::unordered_map<ListenerId, std::function<void(const InputEvent&)>> m_subscribers;
    std::vector<IInputListener*>                                           m_listenerPointers;
    ListenerId                                                             m_nextListenerId = 1;

public:
    ~SInputManager();

    // Delete copy and move constructors/assignment operators
    SInputManager(const SInputManager&)            = delete;
    SInputManager(SInputManager&&)                 = delete;
    SInputManager& operator=(const SInputManager&) = delete;
    SInputManager& operator=(SInputManager&&)      = delete;

    static SInputManager& instance();

    void initialize(sf::RenderWindow* window, bool passToImGui = true);
    void shutdown();

    void update(float deltaTime) override;

    ListenerId subscribe(std::function<void(const InputEvent&)> cb);
    void       unsubscribe(ListenerId id);

    void addListener(IInputListener* listener);
    void removeListener(IInputListener* listener);

    // Query APIs
    bool         isKeyDown(KeyCode key) const;
    bool         wasKeyPressed(KeyCode key) const;
    bool         wasKeyReleased(KeyCode key) const;
    bool         isMouseDown(MouseButton button) const;
    bool         wasMousePressed(MouseButton button) const;
    bool         wasMouseReleased(MouseButton button) const;
    sf::Vector2i getMousePositionWindow() const;

    // Action binding
    BindingId   bindAction(const std::string& actionName, const ActionBinding& binding);
    void        unbindAction(const std::string& actionName, BindingId id);  // remove specific binding
    void        unbindAction(const std::string& actionName);                // remove all bindings
    ActionState getActionState(const std::string& actionName) const;

    // ImGui handling
    void setPassToImGui(bool pass)
    {
        m_passToImGui = pass;
    }

    // Expose event processing for tests
    void processEvent(const sf::Event& event);
};
