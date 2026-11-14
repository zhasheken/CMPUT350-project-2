#pragma once

#include "MathUtil.h"

namespace ECSEngine {


struct CameraShakeComponent {
    int frameLimit{};  // Updates of shake left
    Point2D offset{};  // Shake distance in pixels
};

}  // namespace ECSEngine
