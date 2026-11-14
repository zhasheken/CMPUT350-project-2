#pragma once

#include "MathUtil.h"

namespace ECSEngine {

struct MovementComponent {
    Point2D velocity{};  // Velocity in pixels per update
};

}  // namespace ECSEngine
