#include "Guid.h"
#include <uuid_v4.h>

std::string Guid::generate()
{
    // Use uuid_v4 library for RFC 4122 compliant UUID v4 generation
    static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
    UUIDv4::UUID                                  uuid = uuidGenerator.getUUID();
    return uuid.str();
}
