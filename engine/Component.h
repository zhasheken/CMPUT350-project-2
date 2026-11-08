#pragma once

#include <cassert>
#include <cstddef>
#include <vector>
#include "Entity.h"

namespace ECSEngine {

template <typename T>
class ComponentStorage {
public:
    std::vector<T> mStorage;
    std::vector<bool> mValid;       // Quickly track if a component is valid
    std::vector<EntityID> mOwner;
    std::vector<size_t> mFreeList;  // Avoid scanning for free slots

    size_t Add(EntityID owner, const T& component) {
        // reuse a freed slot first
        if (!mFreeList.empty()) {
            size_t slot = mFreeList.back();
            mFreeList.pop_back();

            mStorage[slot] = component;
            mValid[slot] = true;
            mOwner[slot] = owner;
            return slot;
        }

        // if no free slots, grow all vectors together
        size_t slot = mStorage.size();
        mStorage.push_back(component);
        mValid.push_back(true);
        mOwner.push_back(owner);
        return slot;
    }

    void Remove(size_t slot) {
        assert(slot < mStorage.size());
        assert(mValid[slot]);

        // mark free, dont erase
        mValid[slot] = false;
        mFreeList.push_back(slot);
    }

    // check if slot in use
    bool IsValid(size_t slot) const {
        if (slot >= mValid.size()) {
            return false;
        }
        return mValid[slot];
    }

    // reference invalid after next Add()
    T& Get(size_t slot) {
        assert(IsValid(slot));
        return mStorage[slot];
    }

    EntityID GetOwner(size_t slot) const {
        assert(IsValid(slot));
        return mOwner[slot];
    }

    size_t Size() const {
        return mStorage.size();
    }
};

}  // namespace ECSEngine
