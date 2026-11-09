#pragma once

#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <tuple>
#include <type_traits>
#include <vector>

#include "Component.h"
#include "Entity.h"

namespace ECSEngine {

template <typename... Components>
class EntityManager {
public:
    static constexpr size_t kNullComponent = std::numeric_limits<size_t>::max();
    using tEntity = std::vector<Entity>;
    [[nodiscard]] EntityID CreateEntity(const std::string& name) { return 0; }

    void RemoveEntity(EntityID entity) {}

    template <typename T>
    void AddComponent(EntityID entity, T component) {}

    template <typename T>
    void RemoveComponent(EntityID entity) {}

    template <typename T>
    bool HasComponent(EntityID entity) const {
        return false;
    }

    template <typename T>
    T& GetComponent(EntityID entity) {
        // Re-write code
        static T dummy;
        return dummy;
    }

    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        // Re-write code
        static ComponentStorage<T> dummy;
        return dummy;
    }

private:
    // one slot index per component type
    using tComponentIndices = std::array<size_t, sizeof...(Components)>;

    tEntity mEntities;
    std::vector<tComponentIndices> mComponentIndices;
    std::vector<EntityID> mFreeEntityIDList;
    std::tuple<ComponentStorage<Components>...> mComponentStorages;
};

}  // namespace ECSEngine
