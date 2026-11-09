#pragma once

#include <cstddef>
#include <string>

// An entity can have any component it is templated with,
// but does not have these installed by default.
// This is a base class for all entities in the ECS system.

namespace ECSEngine {

using EntityID = size_t;

class Entity {
private:
    Entity() = default;

    Entity(EntityID eid, const std::string& name) : mID(eid), mType(name), mValid(true) {}

    EntityID mID = 0;
    std::string mType;
    bool mValid = false;

    template <typename... T>
    friend class EntityManager;

public:
    EntityID GetID() const { return mID; }

    const std::string& GetType() const { return mType; }

    // Check if alive
    bool IsValid() const { return mValid; }
};

}  // namespace ECSEngine
