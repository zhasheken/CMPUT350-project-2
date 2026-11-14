#pragma once

#include "MathUtil.h"

namespace ECSEngine {

struct RespawnComponent {
    Point2D loc{};  // Location the entity returns to when it dies
};

}  // namespace ECSEngine
