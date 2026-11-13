#pragma once

#include <cstdint>

#include "MathUtil.h"

namespace ECSEngine {

struct CollisionComponent {
    std::uint8_t staticEntity : 1 = 0;
    std::uint8_t doesDamage : 1 = 0;
    std::uint8_t onGround : 1 = 0;
    std::uint8_t onCeiling : 1 = 0;
    std::uint8_t onLeftWall : 1 = 0;
    std::uint8_t onRightWall : 1 = 0;
    Rect collisionBox{};  // Bounding box relative to LocationComponent::loc
    Rect previousCollisionBox{};  // World space box captured before movement
};

}  // namespace ECSEngine
