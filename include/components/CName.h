#ifndef CNAME_H
#define CNAME_H

#include "Component.h"

namespace Components
{

/**
 * @brief Component for assigning names to entities
 *
 * @description
 * CName is a simple component that provides a string identifier for entities.
 * This is useful for debugging, UI display, and identifying specific entities
 * beyond their tags. The name can be any string and defaults to empty if not
 * specified during construction.
 */
struct CName : public Component
{
public:
    /**
     * @brief Constructs a name component
     * @param n The name to assign (defaults to empty string)
     */
    explicit CName(const std::string& n = "") : m_name(n) {}

    /**
     * @brief Gets the type identifier for this component
     * @return String "Name"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the name data to binary data
     */
    void serialize(Serialization::JsonBuilder& builder) const override;

    /**
     * @brief Deserializes name data from binary data
     */
    void deserialize(const Serialization::SSerialization::JsonValue& value) override;

    /**
     * @brief Gets the name of the entity
     * @return A const reference to the name string
     */
    const std::string& getName() const;

    /**
     * @brief Sets the name of the entity
     * @param n The new name string
     */
    void setName(const std::string& n);

private:
    std::string m_name;  ///< The name string for the entity
};

}  // namespace Components

#endif  // CNAME_H