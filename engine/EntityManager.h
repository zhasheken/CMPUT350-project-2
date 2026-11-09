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
    [[nodiscard]] EntityID CreateEntity(const std::string& name) { 
        tComponentIndices indices;
        indices.fill(kNullComponent);

        // reuse a freed entity ID first
        if (!mFreeEntityIDList.empty()) {
            EntityID entityID = mFreeEntityIDList.back();
            mFreeEntityIDList.pop_back();

            mEntities[entityID] = Entity(entityID, name);
            mComponentIndices[entityID] = indices;
            return entityID;
        }

        // otherwise add it to the end
        EntityID entityID = mEntities.size();
        mEntities.push_back(Entity(entityID, name));
        mComponentIndices.push_back(indices);
        return entityID;

    }

    using const_iterator = typename tEntity::const_iterator;

    void RemoveEntity(EntityID entity) {
        assert(IsEntityIDValid(entity));
        if (!IsEntityIDValid(entity)) {
            return;
        }

        // remove all components
        (RemoveComponentIfPresent<Components>(entity), ...); // once per component type

        // mark entity as invalid and add to free list
        mEntities[entity].mValid = false;
        mFreeEntityIDList.push_back(entity);
    }

    template <typename T>
    void AddComponent(EntityID entity, T component) {
        assert(IsEntityIDValid(entity));
        constexpr size_t index = GetTypeIndex<T>();
        ComponentStorage<T>& storage = std::get<index>(mComponentStorages);
        size_t slot = mComponentIndices[entity][index];

        // if entity already has this component, overwrite it
        if (slot != kNullComponent) {
            storage.Get(slot) = component;
            return;
        }

        // otherwise add it to the storage
        slot = storage.Add(entity, component);
        mComponentIndices[entity][index] = slot;
    }

    template <typename T>
    void RemoveComponent(EntityID entity) {
        assert(hasComponent<T>(entity));
        if (!HasComponent<T>(entity)) {
            return;
        }

        // free the slot and clear index
        constexpr size_t index = GetTypeIndex<T>();
        size_t slot = mComponentIndices[entity][index];
        GetComponentStorage<T>().Remove(slot);
        mComponentIndices[entity][index] = kNullComponent;
    }

    template <typename T>
    bool HasComponent(EntityID entity) const {
        if (!IsEntityIDValid(entity)) {
            return false;
        }
        constexpr size_t index = GetTypeIndex<T>();
        return mComponentIndices[entity][index] != kNullComponent;
    }

    template <typename T>
    T& GetComponent(EntityID entity) {
        assert(HasComponent<T>(entity));
        constexpr size_t index = GetTypeIndex<T>();
        size_t slot = mComponentIndices[entity][index];
        return GetComponentStorage<T>().Get(slot);
    }

    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        constexpr size_t index = GetTypeIndex<T>();
        return std::get<index>(mComponentStorages);
    }

    bool IsEntityIDValid(EntityID entity) const {
        if (entity >= mEntities.size()) {
            return false;
        }
        return mEntities[entity].IsValid();
    }

    const_iterator cbegin() const { return mEntities.begin(); }
    const_iterator cend() const { return mEntities.end(); }
    const_iterator begin() { return mEntities.begin(); }
    const_iterator end() { return mEntities.end(); }

private:
    // one slot index per component type
    using tComponentIndices = std::array<size_t, sizeof...(Components)>;

    tEntity mEntities;
    std::vector<tComponentIndices> mComponentIndices;
    std::vector<EntityID> mFreeEntityIDList;
    std::tuple<ComponentStorage<Components>...> mComponentStorages;

    // position of T, or kNullComponent
    template <typename T>
    static constexpr size_t FindTypeIndex() {
        constexpr bool isMatch[] = {std::is_same_v<T, Components>...}; // copies expression once per component type
        for (size_t i = 0; i < sizeof...(Components); i++) {
            if (isMatch[i]) {
                return i;
            }
        }
        return kNullComponent;
    }

    // errors if T isn't a component
    template <typename T>
    static constexpr size_t GetTypeIndex() {
        constexpr size_t index = FindTypeIndex<T>();
        static_assert(index != kNullComponent, "T is not a component of this EntityManager");
        return index;
    }

    template <typename T>
    void RemoveComponentIfPresent(EntityID entity) {
        if (HasComponent<T>(entity)) {
            RemoveComponent<T>(entity);
        }
    }
};

}  // namespace ECSEngine
