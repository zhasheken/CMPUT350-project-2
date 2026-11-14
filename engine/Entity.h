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
    // Returns the assigned ID; EntityManager may reuse this ID after removal.
    EntityID GetID() const { return mID; }

    // Returns the entity's name/type until the entity is overwritten or destroyed.
    const std::string& GetType() const { return mType; }

    // Reports whether this entity slot is currently alive; a removed ID can later be reused.
    bool IsValid() const { return mValid; }
};

}  // namespace ECSEngine
