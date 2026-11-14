#pragma once

#include <bitset>

#include "Entity.h"
#include "Keyboard.h"

namespace ECSEngine {

struct InputComponent {
    InputComponent(EntityID whichEntity) : entity(whichEntity) {}
    EntityID entity{};
    int lastDash{};
    bool wallSliding{};
    int lastHorizontalPress{};  // -1 left, 1 right
    int wallCoyote{};           // frames wall jump still allowed
    int wallSide{};             // -1 wall on left, 1 right
    std::bitset<Key::ScanCount> keydown{};
    std::bitset<Key::ScanCount> lastKeydown{};
};

}  // namespace ECSEngine
