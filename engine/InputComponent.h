#pragma once

#include <bitset>

#include "Entity.h"
#include "Keyboard.h"

namespace ECSEngine {

struct InputComponent {
    InputComponent(EntityID whichEntity) : entity(whichEntity) {}
    EntityID entity{};
    int lastDash{};
    std::bitset<Key::ScanCount> keydown{};
    std::bitset<Key::ScanCount> lastKeydown{};
};

}  // namespace ECSEngine
