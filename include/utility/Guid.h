#ifndef GUID_H
#define GUID_H

#include <string>

/**
 * @brief Utility class for generating UUID v4 GUIDs
 *
 * @description
 * This class provides functionality to generate UUID version 4 compliant
 * globally unique identifiers in the standard 8-4-4-4-12 hyphenated format.
 * Uses std::random_device and std::mt19937_64 for high-quality randomness.
 */
class Guid
{
public:
    /**
     * @brief Generates a new UUID v4 GUID
     * @return String containing a UUID v4 in format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
     *         where x is any hexadecimal digit and y is one of 8, 9, A, or B
     */
    static std::string generate();

private:
    Guid() = delete;  // Static utility class, prevent instantiation
};

#endif  // GUID_H
