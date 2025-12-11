#include "CInputController.h"
#include <spdlog/spdlog.h>
#include "SInput.h"
#include "SystemLocator.h"

namespace Components
{

CInputController::CInputController() {}

CInputController::~CInputController()
{
    for (const auto& kv : m_bindings)
    {
        const std::string& actionName = kv.first;
        for (const auto& lb : kv.second)
        {
            if (lb.bindingId != 0)
            {
                ::Systems::SystemLocator::input().unbindAction(actionName, lb.bindingId);
            }
        }
    }
    ::Systems::SystemLocator::input().removeListener(this);
}

void CInputController::init()
{
    ::Systems::SystemLocator::input().addListener(this);
}

void CInputController::bindAction(const std::string& actionName, const ActionBinding& binding)
{
    LocalBinding lb;
    lb.binding   = binding;
    lb.bindingId = ::Systems::SystemLocator::input().bindAction(actionName, binding);
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
            ::Systems::SystemLocator::input().unbindAction(actionName, lb.bindingId);
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
    ActionState state = ::Systems::SystemLocator::input().getActionState(actionName);
    return state == ActionState::Held || state == ActionState::Pressed;
}

bool CInputController::wasActionPressed(const std::string& actionName) const
{
    return ::Systems::SystemLocator::input().getActionState(actionName) == ActionState::Pressed;
}

bool CInputController::wasActionReleased(const std::string& actionName) const
{
    return ::Systems::SystemLocator::input().getActionState(actionName) == ActionState::Released;
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

}  // namespace Components
