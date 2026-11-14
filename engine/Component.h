#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

#include "Entity.h"

namespace ECSEngine {

template <typename T>
class ComponentStorage {
public:
    // Components use logical slots: removals mark slots free, and later additions reuse them.
    // References returned by Get can be invalidated if mStorage reallocates as it grows.
    std::vector<T> mStorage;
    std::vector<bool> mValid;  // Quickly track if a component is valid
    std::vector<EntityID> mOwner;
    std::vector<size_t> mFreeList;  // Avoid scanning for free slots

    // Adds a component, reusing the most recently freed slot or appending a new slot.
    // Appending can reallocate mStorage and invalidate references returned by Get.
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

    // Marks a live slot free for reuse; the storage vectors are not compacted.
    void Remove(size_t slot) {
        assert(slot < mStorage.size());
        assert(mValid[slot]);

        // mark free, dont erase
        mValid[slot] = false;
        mFreeList.push_back(slot);
    }

    // Returns false for an out-of-range slot or a slot marked free.
    bool IsValid(size_t slot) const {
        if (slot >= mValid.size()) {
            return false;
        }
        return mValid[slot];
    }

    // Returns a live component by slot; requires IsValid(slot), and may be invalidated by Add.
    T& Get(size_t slot) {
        assert(IsValid(slot));
        return mStorage[slot];
    }

    // Returns the entity ID that currently owns a live slot; requires IsValid(slot).
    EntityID GetOwner(size_t slot) const {
        assert(IsValid(slot));
        return mOwner[slot];
    }

    // Returns allocated slot count, including slots currently marked free.
    size_t Size() const { return mStorage.size(); }
};

}  // namespace ECSEngine
