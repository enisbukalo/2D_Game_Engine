#include "components/CInputController.h"
#include <spdlog/spdlog.h>
#include "systems/SInputManager.h"
#include "utility/JsonBuilder.h"
#include "utility/JsonValue.h"

CInputController::CInputController() {}

CInputController::~CInputController()
{
    // Unbind all registered global bindings
    for (const auto& kv : m_bindings)
    {
        const std::string& actionName = kv.first;
        for (const auto& lb : kv.second)
        {
            if (lb.bindingId != 0)
            {
                SInputManager::instance().unbindAction(actionName, lb.bindingId);
            }
        }
    }
    SInputManager::instance().removeListener(this);
}

void CInputController::init()
{
    SInputManager::instance().addListener(this);
}

std::string CInputController::getType() const
{
    return "CInputController";
}

void CInputController::bindAction(const std::string& actionName, const ActionBinding& binding)
{
    LocalBinding lb;
    lb.binding   = binding;
    lb.bindingId = SInputManager::instance().bindAction(actionName, binding);
    m_bindings[actionName].push_back(lb);
}

void CInputController::unbindAction(const std::string& actionName)
{
    auto it = m_bindings.find(actionName);
    if (it == m_bindings.end())
        return;
    for (const auto& lb : it->second)
    {
        if (lb.bindingId != 0)
            SInputManager::instance().unbindAction(actionName, lb.bindingId);
    }
    m_bindings.erase(it);
    m_callbacks.erase(actionName);
    m_localActionState.erase(actionName);
}

void CInputController::setActionCallback(const std::string& actionName, std::function<void(ActionState)> cb)
{
    if (!cb)
        m_callbacks.erase(actionName);
    else
        m_callbacks[actionName] = cb;
}

bool CInputController::isActionDown(const std::string& actionName) const
{
    ActionState state = SInputManager::instance().getActionState(actionName);
    return state == ActionState::Held || state == ActionState::Pressed;
}

bool CInputController::wasActionPressed(const std::string& actionName) const
{
    return SInputManager::instance().getActionState(actionName) == ActionState::Pressed;
}

bool CInputController::wasActionReleased(const std::string& actionName) const
{
    return SInputManager::instance().getActionState(actionName) == ActionState::Released;
}

void CInputController::onAction(const ActionEvent& ev)
{
    auto it = m_callbacks.find(ev.actionName);
    if (it != m_callbacks.end() && it->second)
    {
        it->second(ev.state);
    }
    m_localActionState[ev.actionName] = ev.state;
}

void CInputController::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cInputController");
    builder.beginObject();
    builder.addKey("actions");
    builder.beginArray();

    for (const auto& kv : m_bindings)
    {
        const std::string& actionName = kv.first;
        const auto&        bindings   = kv.second;
        for (const auto& binding : bindings)
        {
            builder.beginObject();
            builder.addKey("action");
            builder.addString(actionName);

            builder.addKey("keys");
            builder.beginArray();
            for (auto k : binding.binding.keys)
            {
                builder.addString(std::to_string(static_cast<int>(k)));
            }
            builder.endArray();

            builder.addKey("mouse");
            builder.beginArray();
            for (auto m : binding.binding.mouseButtons)
            {
                builder.addNumber(static_cast<int>(m));
            }
            builder.endArray();

            builder.addKey("trigger");
            switch (binding.binding.trigger)
            {
                case ActionTrigger::Pressed:
                    builder.addString("Pressed");
                    break;
                case ActionTrigger::Held:
                    builder.addString("Held");
                    break;
                case ActionTrigger::Released:
                    builder.addString("Released");
                    break;
            }

            builder.addKey("allowRepeat");
            builder.addBool(binding.binding.allowRepeat);
            builder.endObject();
        }
    }

    builder.endArray();
    builder.endObject();
    builder.endObject();
}

void CInputController::deserialize(const JsonValue& value)
{
    const auto& comp = value["cInputController"];
    if (comp.isNull())
        return;
    const auto& actions = comp["actions"].getArray();
    for (size_t i = 0; i < actions.size(); ++i)
    {
        const auto& obj        = actions[i];
        std::string actionName = obj["action"].getString();

        ActionBinding binding;
        const auto&   keysArray = obj["keys"].getArray();
        for (size_t j = 0; j < keysArray.size(); ++j)
        {
            std::string kstr = keysArray[j].getString();
            int         kint = std::stoi(kstr);
            binding.keys.push_back(static_cast<KeyCode>(kint));
        }

        const auto& mouseArray = obj["mouse"].getArray();
        for (size_t j = 0; j < mouseArray.size(); ++j)
        {
            int m = static_cast<int>(mouseArray[j].getNumber());
            binding.mouseButtons.push_back(static_cast<sf::Mouse::Button>(m));
        }

        std::string trigger = obj["trigger"].getString();
        if (trigger == "Pressed")
            binding.trigger = ActionTrigger::Pressed;
        else if (trigger == "Held")
            binding.trigger = ActionTrigger::Held;
        else if (trigger == "Released")
            binding.trigger = ActionTrigger::Released;

        binding.allowRepeat = obj["allowRepeat"].getBool(false);

        bindAction(actionName, binding);
    }
}
