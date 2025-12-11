#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include "Component.h"
#include "Input/ActionBinding.h"
#include "Input/IInputListener.h"
#include "Input/InputEvents.h"

namespace Components
{

class CInputController : public Component, public IInputListener
{
public:
    CInputController();
    ~CInputController();

    // Component lifecycle
    void init();
    void update(float deltaTime) {}

    // Serialization
    void serialize(Serialization::JsonBuilder& builder) const;
    void deserialize(const Serialization::SSerialization::JsonValue& value);

    std::string getType() const;

    // Bindings
    void bindAction(const std::string& actionName, const ActionBinding& binding);
    void unbindAction(const std::string& actionName);

    // Callbacks & queries
    void setActionCallback(const std::string& actionName, std::function<void(ActionState)> cb);
    bool isActionDown(const std::string& actionName) const;
    bool wasActionPressed(const std::string& actionName) const;
    bool wasActionReleased(const std::string& actionName) const;

    // IInputListener overrides
    void onAction(const ActionEvent& ev) override;

private:
    struct LocalBinding
    {
        ActionBinding binding;
        size_t        bindingId{0};
    };
    std::unordered_map<std::string, std::vector<LocalBinding>>        m_bindings;
    std::unordered_map<std::string, std::function<void(ActionState)>> m_callbacks;
    std::unordered_map<std::string, ActionState>                      m_localActionState;
};

}  // namespace Components
