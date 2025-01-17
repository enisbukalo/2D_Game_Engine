#ifndef CNAME_H
#define CNAME_H

#include "Component.h"

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
#pragma region Variables
    std::string name;  ///< The name string for the entity
#pragma endregion

#pragma region Constructors
    /**
     * @brief Constructs a name component
     * @param n The name to assign (defaults to empty string)
     */
    explicit CName(const std::string& n = "") : name(n) {}
#pragma endregion

#pragma region Override Methods
    /**
     * @brief Gets the type identifier for this component
     * @return String "Name"
     */
    std::string getType() const override;

    /**
     * @brief Serializes the name data to JSON
     * @return JSON object containing the name string
     */
    json serialize() const override;

    /**
     * @brief Deserializes name data from JSON
     * @param data JSON object containing name data
     */
    void deserialize(const json& data) override;
#pragma endregion
};

#endif  // CNAME_H