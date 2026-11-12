#pragma once

#include <bitset>
#include <unordered_map>

#include "Keyboard.h"
#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace ECSEngine {

struct InputComponent {
    InputComponent(EntityID whichEntity) : entity(whichEntity), lastDash(0) {}
    EntityID entity;
    int lastDash;
    std::bitset<Key::ScanCount> keydown;
    std::bitset<Key::ScanCount> lastKeydown;
};

}  // namespace ECSEngine
