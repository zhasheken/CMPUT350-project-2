#pragma once

#include <bitset>

#include "Entity.h"
#include "Keyboard.h"

namespace ECSEngine {

struct InputComponent {
    // Associates the input state with its player entity.
    InputComponent(EntityID whichEntity) : entity(whichEntity) {}
    EntityID entity{};
    int lastDash{};
    bool wallSliding{};
    std::bitset<Key::ScanCount> keydown{};
    std::bitset<Key::ScanCount> lastKeydown{};
};

}  // namespace ECSEngine
