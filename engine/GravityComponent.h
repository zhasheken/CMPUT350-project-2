#pragma once

#include "MathUtil.h"

namespace ECSEngine {

struct GravityComponent {
    // Stores discrete acceleration and a timer value.
    GravityComponent(Point2D acceleration, int disabledTimer = 0)
        : acceleration(acceleration), disabledTimer(disabledTimer) {}

    Point2D acceleration{};  // Acceleration in pixels per update squared
    int disabledTimer{};
};

}  // namespace ECSEngine
