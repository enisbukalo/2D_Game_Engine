#ifndef CNAME_H
#define CNAME_H

#include <string>
#include "SSerialization.h"

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
struct CName
{
    explicit CName(const std::string& n = "") : name(n) {}

    std::string        getType() const;
    void               serialize(Serialization::JsonBuilder& builder) const;
    void               deserialize(const Serialization::SSerialization::JsonValue& value);
    const std::string& getName() const;
    void               setName(const std::string& n);

    std::string name;
};

}  // namespace Components

#endif  // CNAME_H