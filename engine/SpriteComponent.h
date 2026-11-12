#pragma once

#include "MathUtil.h"
#include "SpriteManager.h"

namespace ECSEngine {

// The sprite rectangle is relative to the entity location in the world
// This is only defined if an entity also has a LocationComponent.
class SpriteComponent {
public:
    SpriteID textureID{};
    Rect bounds{};
    bool worldSpace{};  // If false, this sprite is drawn in screen space instead of world space
};

}  // namespace ECSEngine
