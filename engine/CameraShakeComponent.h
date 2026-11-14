#pragma once

#include "MathUtil.h"

namespace ECSEngine {


struct CameraShakeComponent {
    int frameLimit{};
    Point2D offset{};
};

}  // namespace ECSEngine
